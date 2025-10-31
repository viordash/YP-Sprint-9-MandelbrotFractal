#include <gtest/gtest.h>
#include <stdexec/execution.hpp>

#include "mandelbrot.hpp"
#include "types.hpp"

TEST(CalculateMandelbrotAsyncSenderTest, Do_Render_When_Needed) {
    const std::uint32_t width = 320;
    const std::uint32_t height = 240;
    const std::uint32_t max_iter = 100;

    RenderSettings settings{.width = width, .height = height, .max_iterations = max_iter, .escape_radius = 2.0};

    AppState state;
    state.need_rerender = true;

    MandelbrotRenderer renderer;

    CalculateMandelbrotAsyncSender sender{state, settings, renderer};
    auto result = stdexec::sync_wait(std::move(sender));

    ASSERT_TRUE(result.has_value());
    auto &[render_result] = result.value();

    ASSERT_FALSE(state.need_rerender);

    ASSERT_FALSE(render_result.color_data.empty());
    ASSERT_EQ(render_result.color_data.size(), settings.height);
    ASSERT_GT(render_result.render_time.count(), 0);
}

TEST(CalculateMandelbrotAsyncSenderTest, Skips_Render_When_Not_Needed) {
    RenderSettings settings;

    AppState state;
    state.need_rerender = false;

    MandelbrotRenderer renderer;

    CalculateMandelbrotAsyncSender sender{state, settings, renderer};
    auto result = stdexec::sync_wait(std::move(sender));

    ASSERT_TRUE(result.has_value());
    auto &[render_result] = result.value();

    ASSERT_TRUE(render_result.color_data.empty());
    ASSERT_FALSE(state.need_rerender);
}