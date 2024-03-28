#include "Framebuffer.h"
#include "UI.h"
#include <chrono>
#include <algorithm>
#include "Font.h"
#include <numeric>
#include "Syslog.h"
#include "Window.h"
#include <deque>

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

    const size_t maxMessagesOnScreen = 20;
    std::deque<ogl::Text*> consoleMsg;

    float textOffset = 0.01f;
}

void ogl::Performance::init(){
    //font = new ogl::Font("forTests/font/ui_font_hud_01.dds", "forTests/font/ui_font_hud_01.ini");
    font = new ogl::Font("forTests/font/ui_font_console_02.dds", "forTests/font/ui_font_console_02.ini");
    textOffset = float(font->getSymbolHeight())/1080.f; // WARNING привязка к фулл хд, убрать!

    fpsText = new ogl::Text();
    fpsText->setFont(*font);
    fpsText->setText("Fps: " + std::to_string(0.f));
    fpsText->setPosition(0.01f, 0.01f + textOffset);
    fpsText->setSmooth(false);

    nPercentLowText = new ogl::Text();
    nPercentLowText->setFont(*font);
    nPercentLowText->setText(percentLowText + std::to_string(0.f));
    nPercentLowText->setPosition(0.01f, 0.01f);
    nPercentLowText->setSmooth(false);
}

void ogl::Performance::render() {
    ogl::Framebuffer::saveBindings();
    ogl::Window::getFramebuffer()->bindForDraw();

    auto newMsg = Syslog::newMessagesAmount();
    for (size_t i = 0; i < newMsg; i++) {
        for (auto m : consoleMsg)
            m->move(0.f, textOffset);

        consoleMsg.push_back(new ogl::Text());
        consoleMsg.back()->setFont(*font);
        consoleMsg.back()->setText(Syslog::getMessage(newMsg - i - 1)->getSummary());
        consoleMsg.back()->setSmooth(false);
        consoleMsg.back()->setPosition(0.01f, 0.8f);
        consoleMsg.back()->setColor(Syslog::getMessage(newMsg - i - 1)->getColor());

        if (consoleMsg.size() >= maxMessagesOnScreen) {
            delete consoleMsg[0];
            consoleMsg.pop_front();
        }
    }

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

        for (auto m : consoleMsg)
            m->copyToBinded();
    }

    ogl::Framebuffer::restoreBindings();
}
