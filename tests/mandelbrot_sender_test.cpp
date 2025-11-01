#include <gtest/gtest.h>
#include <limits>
#include <stdexcept>
#include <vector>

#include "mandelbrot_sender.hpp"
#include "types.hpp"

TEST(MandelbrotSenderTest, Calculates_Correctly) {
    const std::uint32_t width = 3;
    const std::uint32_t height = 3;
    const std::uint32_t max_iter = 100;

    RenderSettings settings{.width = width, .height = height, .max_iterations = max_iter, .escape_radius = 2.0};

    mandelbrot::ViewPort viewport{.x_min = -1.5, .x_max = 1.5, .y_min = -1.5, .y_max = 1.5};

    PixelRegion region{.start_row = 0, .end_row = 3, .start_col = 0, .end_col = 3};

    MandelbrotSender sender{viewport, settings, region};
    auto result = stdexec::sync_wait(std::move(sender));

    std::tuple<std::vector<std::vector<unsigned int>>> ddd;

    ASSERT_TRUE(result.has_value());
    auto &[matrix] = result.value();

    ASSERT_EQ(matrix.size(), width);
    ASSERT_EQ(matrix[0].size(), height);

    const auto &center_pixel = matrix[1][1];
    ASSERT_EQ(center_pixel, settings.max_iterations);

    const auto &corner_pixel = matrix[0][0];
    ASSERT_EQ(corner_pixel, 1);
}

TEST(MandelbrotSenderTest, Handles_Exceptions) {
    RenderSettings settings{.width = 1, .height = 1, .max_iterations = 1};
    mandelbrot::ViewPort viewport;

    PixelRegion bad_alloc_region{
        .start_row = 0, .end_row = std::numeric_limits<uint32_t>::max(), .start_col = 0, .end_col = 1};

    MandelbrotSender sender{viewport, settings, bad_alloc_region};

    try {
        stdexec::sync_wait(std::move(sender));
        FAIL() << "No exception was thrown";
    } catch (const std::bad_alloc &) {
        SUCCEED();
    } catch (const std::exception &e) {
        FAIL() << e.what();
    } catch (...) {
        FAIL() << "Expected std::bad_alloc";
    }
}