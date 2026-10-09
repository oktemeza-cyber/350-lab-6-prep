#include <iostream>

#include "waldo_data.h"
#include "zoom.h"

ZoomApp::ZoomApp(float maxWorldWidth, float maxWorldHeight) : mIsZooming(false) {
    // Load the Waldo image and determine the world size (mWorldSize)
    loadWorld(maxWorldWidth, maxWorldHeight);
    // Set up the default world view
    mWorldViewDefault.setSize(mWorldSize);
    mWorldViewDefault.setCenter(mWorldSize * 0.5f);
    mWorldViewZoomed.setSize(mWorldSize / ZOOM_FACTOR);
    mWorldViewZoomed.setCenter(mWorldSize * 0.5f);
    // Set window
    mWindow = sf::RenderWindow(
        sf::VideoMode(sf::Vector2u(static_cast<int>(mWorldSize.x), static_cast<int>(mWorldSize.y))),
        "Zoom App");
    // Set initial viewport
    updateViewAfterResize();
    mWindow.setView(mWorldViewDefault);
}

void ZoomApp::run() {
    while (mWindow.isOpen()) {
        // handle inputs
        while (const std::optional<sf::Event> event = mWindow.pollEvent()) {
            handleEvent(*event);
        }
        // draw
        render();
    }
}

// loadWorld loads the waldo image into the waldo texture, sets the waldo sprite,
// and computs the world size based on the largest dimensions which fit into
// { maxWorldWidth, maxWorldHeight } that maintain the texture's original aspect ratio.
void ZoomApp::loadWorld(float maxWorldWidth, float maxWorldHeight) {
    if (!mWaldoTexture.loadFromMemory(EmbeddedImages::waldo_data, EmbeddedImages::waldo_size)) {
        throw std::runtime_error("Failed to load Waldo image from embedded data");
    }
    mWaldoSprite = std::make_unique<sf::Sprite>(mWaldoTexture);
    sf::Vector2u textureSize = mWaldoTexture.getSize();
    // Calculate scale to fit the image within the world bounds while maintaining aspect ratio
    float scaleX = maxWorldWidth / static_cast<float>(textureSize.x);
    float scaleY = maxWorldHeight / static_cast<float>(textureSize.y);
    float scale = std::min(scaleX, scaleY);
    // The size of the world is just the resulting size of the texture after rescaling it
    // to just fit within max world size.
    mWorldSize = {scale * textureSize.x, scale * textureSize.y};
    // Apply the scale transform that makes mWaldoSprite of size mWorldSize:
    mWaldoSprite->setScale(sf::Vector2f(scale, scale));
}

void ZoomApp::handleEvent(const sf::Event& event) {
    // Update ideally should be modularized from input handling, but keeping things simple for
    // this lab:
    if (event.is<sf::Event::Closed>()) {
        mWindow.close();
    } else if (event.is<sf::Event::Resized>()) {
        updateViewAfterResize();
    } else if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Space) {
            mIsZooming = true;
        }
    } else if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>()) {
        if (keyReleased->code == sf::Keyboard::Key::Space) {
            mIsZooming = false;
        }
    } else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        updateZoomView(mouseMoved->position);
    }
}

// updateViewAfterResize updates BOTH world views so that it renders into the largest possible
// centered rectangle with the same aspect ratio as mWorldSize, that still shows the entire
// world when unzoomed.
void ZoomApp::updateViewAfterResize() {

    float worldRatio = static_cast<float>(mWorldSize.x) / static_cast<float>(mWorldSize.y);

    sf::Vector2u windowSize = mWindow.getSize();
    float windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);

    float width = 1.0f;
    float height = 1.0f;

    float left = (1.0f - width) / 2.0f;
    float top = (1.0f - height) / 2.0f;

    sf::FloatRect newViewport({left, top}, {width, height});

    mWorldViewDefault.setViewport(newViewport);
    mWorldViewZoomed.setViewport(newViewport);

}

// updateZoomView sets the center of mWorldViewZoomed such that the zoomed view would have the
// user's cursor pointing to the same thing as when it is unzoomed.
void ZoomApp::updateZoomView(sf::Vector2i mousePos) {
    // TODO: Implement this method. High-level outline/hints given.
    // (1) Get position of mouse relative to viewport's top-left in window pixel units.
    //     Remember that in SFML, viewports' position and size are both given as PERCENTAGES
    //     of the window size.
    // (2) Get position of mouse relative to viewport's top-left as percentage of original
    //     viewport.
    // (3) Then use that to get pos of mouse relative to world top-left in world units.
    // (4) Derive the new viewport center (in world units)
    //     which keeps the mouse position pointing at the same thing in
    //     original image but now within a world-space rectangle of size
    //     mWorldSize / ZOOM_FACTOR.
    //  HINT: Determine the steps to go from the desired center to the top-left of this new
    //        world-space rectangle, then from this top-left to the thing you're pointing at,
    //        using our above computed vars. Then solve the equation for the desired center.

    sf::Vector2u windowSize = mWindow.getSize();
    sf::FloatRect viewPort = mWorldViewDefault.getViewport();

    float topLeftX = mousePos.x - viewPort.position.x * windowSize.x;
    float topLeftY = mousePos.y - viewPort.position.y * windowSize.y;

    float percentX = topLeftX / (viewPort.size.x * windowSize.x);
    float percentY = topLeftY / (viewPort.size.y * windowSize.y);

    float worldX = percentX * mWorldSize.x;
    float worldY = percentY * mWorldSize.y;

    float zoomW = mWorldSize.x / ZOOM_FACTOR;
    float zoomH = mWorldSize.y / ZOOM_FACTOR;

    float centerX = worldX + zoomW * (0.5f - percentX);
    float centerY = worldY + zoomH * (0.5f - percentY);

    mWorldViewZoomed.setCenter({centerX, centerY});
}

void ZoomApp::render() {
    mWindow.clear(sf::Color::Black);
    if (!mIsZooming) {
        mWindow.setView(mWorldViewDefault);
    } else {
        mWindow.setView(mWorldViewZoomed);
    }
    mWindow.draw(*mWaldoSprite);
    mWindow.display();
}

int main() {
    try {
        ZoomApp app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
