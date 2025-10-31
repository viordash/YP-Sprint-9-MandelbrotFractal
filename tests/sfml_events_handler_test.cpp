#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdexec/execution.hpp>
#include <stdexec/functional.hpp>

#include "sfml_events_handler.hpp"
#include "types.hpp"

struct TestSinkReceiver {
    using receiver_concept = stdexec::receiver_t;
    void set_value(int value) noexcept {}
    void set_error(std::exception_ptr e) noexcept {}
    void set_stopped() noexcept {}
};

class TestableOperationState : public SfmlEventHandler::OperationState<TestSinkReceiver> {
public:
    using SfmlEventHandler::OperationState<TestSinkReceiver>::OperationState;

    MOCK_METHOD(bool, pollEvent, (sf::Event & event), (override));

    void PublicMorozov_ZoomToPoint(int x, int y, bool in, double factor = 0.8) { this->ZoomToPoint(x, y, in, factor); }

    void PublicMorozov_HandleEvents() { this->HandleEvents(); }
};

ACTION_P(SetEvent, event) {
    arg0 = event;
    return true;
}

TEST(SfmlEventHandlerTest, Handles_Window_Close_Event) {
    AppState state;
    RenderSettings settings;
    sf::Clock clock;
    sf::RenderWindow window;

    TestableOperationState test_op_state(TestSinkReceiver{}, window, settings, state, clock);

    sf::Event close_event;
    close_event.type = sf::Event::Closed;
    EXPECT_CALL(test_op_state, pollEvent).WillOnce(SetEvent(close_event)).WillOnce(testing::Return(false));

    state.should_exit = false;
    test_op_state.PublicMorozov_HandleEvents();

    ASSERT_TRUE(state.should_exit);
}

TEST(SfmlEventHandlerTest, Handles_Mouse_Button_Events) {
    AppState state;
    RenderSettings settings;
    sf::Clock clock;
    sf::RenderWindow window;

    TestableOperationState test_op_state(TestSinkReceiver{}, window, settings, state, clock);

    sf::Event lmouse_press_event;
    lmouse_press_event.type = sf::Event::MouseButtonPressed;
    lmouse_press_event.mouseButton.button = sf::Mouse::Left;

    sf::Event lmouse_release_event;
    lmouse_release_event.type = sf::Event::MouseButtonReleased;
    lmouse_release_event.mouseButton.button = sf::Mouse::Left;

    sf::Event rmouse_press_event;
    rmouse_press_event.type = sf::Event::MouseButtonPressed;
    rmouse_press_event.mouseButton.button = sf::Mouse::Right;

    sf::Event rmouse_release_event;
    rmouse_release_event.type = sf::Event::MouseButtonReleased;
    rmouse_release_event.mouseButton.button = sf::Mouse::Right;

    EXPECT_CALL(test_op_state, pollEvent)
        .WillOnce(SetEvent(lmouse_press_event))
        .WillOnce(testing::Return(false))
        .WillOnce(SetEvent(lmouse_release_event))
        .WillOnce(testing::Return(false))
        .WillOnce(SetEvent(rmouse_press_event))
        .WillOnce(testing::Return(false))
        .WillOnce(SetEvent(rmouse_release_event))
        .WillOnce(testing::Return(false));

    test_op_state.PublicMorozov_HandleEvents();
    ASSERT_TRUE(state.left_mouse_pressed);
    test_op_state.PublicMorozov_HandleEvents();
    ASSERT_FALSE(state.left_mouse_pressed);

    test_op_state.PublicMorozov_HandleEvents();
    ASSERT_TRUE(state.right_mouse_pressed);
    test_op_state.PublicMorozov_HandleEvents();
    ASSERT_FALSE(state.right_mouse_pressed);
}

TEST(SfmlEventHandlerTest, ZoomToPoint) {
    AppState state;
    state.viewport = {-2.0, 2.0, -2.0, 2.0};
    RenderSettings settings{800, 600};
    sf::Clock clock;
    sf::RenderWindow window;

    TestableOperationState test_op_state(TestSinkReceiver{}, window, settings, state, clock);

    test_op_state.PublicMorozov_ZoomToPoint(400, 300, true, 0.5);

    ASSERT_TRUE(state.need_rerender);
    EXPECT_NEAR(state.viewport.width(), 2.0, 1e-9);
    EXPECT_NEAR(state.viewport.height(), 2.0, 1e-9);
    EXPECT_NEAR(state.viewport.x_min, -1.0, 1e-9);
    EXPECT_NEAR(state.viewport.x_max, 1.0, 1e-9);
    EXPECT_NEAR(state.viewport.y_min, -1.0, 1e-9);
    EXPECT_NEAR(state.viewport.y_max, 1.0, 1e-9);
}