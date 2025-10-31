#pragma once

#include <SFML/Graphics.hpp>
#include <exception>
#include <print>
#include <stdexec/execution.hpp>
#include <utility>

#include "types.hpp"

class SFMLRender {
public:
    using sender_concept = stdexec::sender_t;

    using completion_signatures = stdexec::completion_signatures<  //
        stdexec::set_value_t(),                                    //
        stdexec::set_error_t(std::exception_ptr),                  //
        stdexec::set_stopped_t()                                   //
        >;

    template <typename Receiver>
    struct OperationState {
        Receiver receiver_;
        RenderResult render_result_;
        sf::Image &image_;
        sf::Texture &texture_;
        sf::Sprite &sprite_;
        sf::RenderWindow &window_;
        RenderSettings render_settings_;

        template <typename R>
        OperationState(R &&rec, RenderResult res, sf::Image &img, sf::Texture &tex, sf::Sprite &spr,
                       sf::RenderWindow &win, RenderSettings settings)
            : receiver_(std::forward<R>(rec)), render_result_(std::move(res)), image_{img}, texture_{tex}, sprite_{spr},
              window_{win}, render_settings_{settings} {}

        friend void tag_invoke(stdexec::start_t, OperationState &self) noexcept {
            try {
                if (self.render_result_.color_data.empty()) {
                    stdexec::set_value(std::move(self.receiver_));
                    return;
                }

                for (auto y = 0; y < self.render_settings_.height; ++y) {
                    for (auto x = 0; x < self.render_settings_.width; ++x) {
                        const auto &color = self.render_result_.color_data[y][x];
                        self.image_.setPixel(x, y, sf::Color(color.r, color.g, color.b));
                    }
                }

                self.texture_.update(self.image_);
                self.sprite_.setTexture(self.texture_);

                self.window_.clear();
                self.window_.draw(self.sprite_);
                self.window_.display();

                stdexec::set_value(std::move(self.receiver_));
            } catch (...) {
                stdexec::set_error(std::move(self.receiver_), std::current_exception());
            }
        }
    };

    RenderResult render_result_;
    sf::Image &image_;
    sf::Texture &texture_;
    sf::Sprite &sprite_;
    sf::RenderWindow &window_;
    RenderSettings render_settings_;

    SFMLRender(RenderResult render_result, sf::Image &image, sf::Texture &texture, sf::Sprite &sprite,
               sf::RenderWindow &window, RenderSettings render_settings)
        : render_result_(std::move(render_result)), image_{image}, texture_{texture}, sprite_{sprite}, window_{window},
          render_settings_{render_settings} {}

    template <stdexec::receiver Receiver>
    friend auto tag_invoke(stdexec::connect_t, SFMLRender &&self, Receiver &&receiver)
        -> OperationState<std::decay_t<Receiver>> {
        return OperationState<std::decay_t<Receiver>>{std::forward<Receiver>(receiver),
                                                      std::move(self.render_result_),
                                                      self.image_,
                                                      self.texture_,
                                                      self.sprite_,
                                                      self.window_,
                                                      self.render_settings_};
    }
};