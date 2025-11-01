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

TEST(CalculateMandelbrotAsyncSenderTest, Handles_Invalid_Settings) {
    AppState state;
    state.need_rerender = true;

    MandelbrotRenderer renderer;

    {
        RenderSettings settings_zero_width = {.width = 0, .height = 100};
        CalculateMandelbrotAsyncSender sender{state, settings_zero_width, renderer};
        EXPECT_NO_THROW(stdexec::sync_wait(std::move(sender)));
    }

    {
        RenderSettings settings_zero_height = {.width = 100, .height = 0};
        CalculateMandelbrotAsyncSender sender{state, settings_zero_height, renderer};
        EXPECT_NO_THROW(stdexec::sync_wait(std::move(sender)));
    }

    {
        RenderSettings settings_large_size = {.width = 10000, .height = 10000, .max_iterations = 1};
        CalculateMandelbrotAsyncSender sender{state, settings_large_size, renderer};
        EXPECT_NO_THROW(stdexec::sync_wait(std::move(sender)));
    }
}