#include "sfml_renderer.hpp"
#include "types.hpp"
#include <SFML/Graphics.hpp>
#include <gtest/gtest.h>

TEST(SFMLRenderTest, Start_Renders) {
    RenderResult result;
    result.color_data = {{mandelbrot::RgbColor{255, 0, 0}, mandelbrot::RgbColor{0, 255, 0}}};

    RenderSettings settings;
    settings.width = result.color_data[0].size();
    settings.height = 1;

    sf::Image image;
    image.create(settings.width, settings.height, sf::Color::Black);
    sf::Texture texture;
    texture.create(settings.width, settings.height);
    sf::Sprite sprite;
    sf::RenderWindow window;

    SFMLRender sender{std::move(result), image, texture, sprite, window, settings};
    stdexec::sync_wait(std::move(sender));

    ASSERT_EQ(image.getPixel(0, 0), sf::Color(255, 0, 0));
    ASSERT_EQ(image.getPixel(1, 0), sf::Color(0, 255, 0));
}

TEST(SFMLRenderTest, Skip_Rendering_For_Empty_Data) {
    RenderResult result;
    RenderSettings settings;
    settings.width = 2;
    settings.height = 1;

    sf::Image image;
    image.create(settings.width, settings.height, sf::Color::Black);
    sf::Texture texture;
    texture.create(settings.width, settings.height);
    sf::Sprite sprite;
    sf::RenderWindow window;

    SFMLRender sender{std::move(result), image, texture, sprite, window, settings};
    stdexec::sync_wait(std::move(sender));

    ASSERT_EQ(image.getPixel(0, 0), sf::Color(0, 0, 0));
    ASSERT_EQ(image.getPixel(1, 0), sf::Color(0, 0, 0));
}
