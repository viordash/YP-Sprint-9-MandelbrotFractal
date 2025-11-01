#include <gtest/gtest.h>
#include <vector>

#include "mandelbrot_renderer.hpp"
#include "types.hpp"

TEST(MandelbrotRendererTest, Renders_And_Strips_Combines_Correctly) {

    const std::uint32_t width = 320;
    const std::uint32_t height = 240;
    const std::uint32_t max_iter = 100;
    const size_t N = 4;

    RenderSettings settings{.width = width, .height = height, .max_iterations = max_iter, .escape_radius = 2.0};

    mandelbrot::ViewPort viewport{.x_min = -2.0, .x_max = 1.0, .y_min = -1.0, .y_max = 1.0};

    MandelbrotRenderer renderer{N};

    auto render_sender = renderer.RenderAsync<N>(viewport, settings);
    auto result = stdexec::sync_wait(std::move(render_sender));

    ASSERT_TRUE(result.has_value());
    auto &[render_result] = result.value();

    ASSERT_EQ(render_result.color_data.size(), height);
    ASSERT_EQ(render_result.color_data[0].size(), width);

    ASSERT_EQ(render_result.settings.width, settings.width);
    ASSERT_EQ(render_result.settings.height, settings.height);
    ASSERT_GT(render_result.render_time.count(), 0);

    auto expected_color = [&](const std::uint32_t x, const std::uint32_t y) {
        const mandelbrot::Complex c = mandelbrot::Pixel2DToComplex(x, y, viewport, width, height);
        const std::uint32_t iterations = mandelbrot::CalculateIterationsForPoint(c, max_iter, 2.0);
        return mandelbrot::IterationsToColor(iterations, max_iter);
    };
    for (std::uint32_t y = 0; y < height; ++y) {
        for (std::uint32_t x = 0; x < width; ++x) {
            ASSERT_EQ(render_result.color_data[y][x], expected_color(x, y));
        }
    }
}