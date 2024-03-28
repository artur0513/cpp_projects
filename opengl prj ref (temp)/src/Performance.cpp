#include "Framebuffer.h"
#include "Performance.h"
#include <chrono>
#include <algorithm>
#include "Font.h"
#include <numeric>
#include "Window.h"

using namespace std::chrono_literals;

m3d::vec4f fpsToColor(float fps) {
    return m3d::vec4f(std::clamp(1.33f - fps / 40.f, 0.0f, 1.0f), std::clamp(fps / 40.f - 0.33f, 0.0f, 1.0f), 0.0f, 1.0f);
}

namespace {
    bool showPerformance = true;
    std::chrono::steady_clock::time_point fpsClock = std::chrono::steady_clock::now(); // for calculating avg fps
    std::chrono::steady_clock::time_point frameClock = std::chrono::steady_clock::now(); // for calculating frame times

    constexpr size_t percentLow = 5;
    std::string percentLowText = std::to_string(percentLow) + "\% low: ";
    constexpr size_t framesToAnalize = 300;
    constexpr size_t nPercentFrames = framesToAnalize * percentLow / 100;
    std::vector<float> frameTimes(framesToAnalize, 0.f), copyVec(framesToAnalize);
    float nPercentLow = 0.f;
    size_t nLowFrameCount = 0;

    float fps = 0.f;
    auto fpsRefreshTime = 1s;
    size_t fpsFrameCounter = 0;

    ogl::Text *fpsText, *nPercentLowText;
    ogl::Font *font;
}

void ogl::Performance::init(){
    font = new ogl::Font("forTests/font/ui_font_hud_01.dds", "forTests/font/ui_font_hud_01.ini");

    fpsText = new ogl::Text();
    fpsText->setFont(*font);
    fpsText->setText("Fps: " + std::to_string(0.f));
    fpsText->setPosition(m3d::vec2f(0.01f, 0.02f));
    fpsText->setSmooth(false);

    nPercentLowText = new ogl::Text();
    nPercentLowText->setFont(*font);
    nPercentLowText->setText(percentLowText + std::to_string(0.f));
    nPercentLowText->setPosition(m3d::vec2f(0.01f, 0.01f));
    nPercentLowText->setSmooth(false);
}

void ogl::Performance::render() {
    ogl::Framebuffer::saveBindings();
    ogl::Window::getFramebuffer()->bindForDraw();

    // 1% LOW PART
    auto frameTime = std::chrono::duration<float>(std::chrono::steady_clock::now() - frameClock).count();
    frameClock = std::chrono::steady_clock::now();
    if (nLowFrameCount < framesToAnalize) {
        frameTimes[nLowFrameCount] = frameTime;
        nLowFrameCount++;
    }
    else
        nLowFrameCount = 0;

    // FPS PART
    fpsFrameCounter++;
    if ((std::chrono::steady_clock::now() - fpsClock) > fpsRefreshTime) {
        std::chrono::duration<float> time = std::chrono::steady_clock::now() - fpsClock;
        fpsClock = std::chrono::steady_clock::now();
        fps = float(fpsFrameCounter) / time.count();

        fpsFrameCounter = 0;
        fpsText->setText("Fps: " + std::to_string(fps));

        copy(frameTimes.begin(), frameTimes.end(), copyVec.begin());
        std::partial_sort(copyVec.begin(), copyVec.begin() + nPercentFrames + 1, copyVec.begin() + framesToAnalize, std::greater{});
        nPercentLow = float(nPercentFrames) / std::accumulate(copyVec.begin(), copyVec.begin() + nPercentFrames + 1, 0.f);
        nPercentLowText->setText(percentLowText + std::to_string(nPercentLow));

        nPercentLowText->setColor(fpsToColor(nPercentLow));
        fpsText->setColor(fpsToColor(fps));
    }

    if (showPerformance){
        fpsText->copyToBinded();
        nPercentLowText->copyToBinded();
    }

    ogl::Framebuffer::restoreBindings();
}
