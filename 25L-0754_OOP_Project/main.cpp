#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <SFML/Graphics.hpp>
#include "FileHandler.h"
#include "Utils.h"
#include "Storage.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include "Exceptions.h"
#include "Validator.h"
using namespace std;

// ==================== HELPER ====================

void getCurrentDate(char* date) {
    time_t now = time(nullptr);
    tm* local = localtime(&now);
    int day = local->tm_mday;
    int month = local->tm_mon + 1;
    int year = local->tm_year + 1900;
    date[0] = (day / 10) + '0';
    date[1] = (day % 10) + '0';
    date[2] = '-';
    date[3] = (month / 10) + '0';
    date[4] = (month % 10) + '0';
    date[5] = '-';
    date[6] = (year / 1000) + '0';
    date[7] = ((year / 100) % 10) + '0';
    date[8] = ((year / 10) % 10) + '0';
    date[9] = (year % 10) + '0';
    date[10] = '\0';
}

bool isDateAfter(const char* dateA, const char* dateB) {
    int dayA = (dateA[0] - '0') * 10 + (dateA[1] - '0');
    int monthA = (dateA[3] - '0') * 10 + (dateA[4] - '0');
    int yearA = (dateA[6] - '0') * 1000 + (dateA[7] - '0') * 100 + (dateA[8] - '0') * 10 + (dateA[9] - '0');

    int dayB = (dateB[0] - '0') * 10 + (dateB[1] - '0');
    int monthB = (dateB[3] - '0') * 10 + (dateB[4] - '0');
    int yearB = (dateB[6] - '0') * 1000 + (dateB[7] - '0') * 100 + (dateB[8] - '0') * 10 + (dateB[9] - '0');

    if (yearA != yearB) return yearA > yearB;
    if (monthA != monthB) return monthA > monthB;
    return dayA > dayB;
}

void sfStringToCharArray(const sf::String& input, char* output, int maxLen) {
    int i = 0;
    while (i < maxLen - 1 && i < (int)input.getSize()) {
        sf::Uint32 code = input[i];
        output[i] = (code <= 127) ? (char)code : '?';
        i++;
    }
    output[i] = '\0';
}

bool loadUIFont(sf::Font& font) {
    const char* fontPaths[] = {
        "Asset/fonts/arial.ttf",
        "Asset/fonts/arial1.ttf",
        "C:/Windows/Fonts/arial.ttf"
    };
    for (int i = 0; i < 3; i++) {
        if (font.loadFromFile(fontPaths[i])) return true;
    }
    return false;
}

bool loadThemeTexture(sf::Texture& texture, const char* paths[], int count);
void showVisualMessage(const char* title, const char* message);
bool registerPatientVisual(Storage<Patient>& patients);

/* One size and one asset list for every SFML screen (main, login, role menus, popups). */
static const int APP_WIN_W = 1024;
static const int APP_WIN_H = 700;

static const char* THEME_BG_PATHS[] = {
    "Asset/Image/HospitalBG.png",
    "Asset/Image/HospitalBG.jpg",
    "Asset/Image/HospitalBackground.png",
    "Asset/Image/HospitalBackground.jpg",
    "Asset/Image/HospitalMain.png",
    "Asset/Image/HospitalMain.jpg",
    "Asset/Image/Hospital.jpg",
    "Asset/Image/Hospital.png"
};
static const int THEME_BG_COUNT = 8;

static void themeApplyBackgroundCover(sf::Sprite& spr, const sf::Texture& tex, int winW, int winH) {
    sf::Vector2u szu = tex.getSize();
    if (szu.x == 0 || szu.y == 0) return;
    float tw = (float)szu.x;
    float th = (float)szu.y;
    float sx = (float)winW / tw;
    float sy = (float)winH / th;
    float s = (sx > sy) ? sx : sy;
    spr.setTexture(tex);
    spr.setScale(s, s);
    float dw = tw * s;
    float dh = th * s;
    spr.setPosition((winW - dw) * 0.5f, (winH - dh) * 0.5f);
}

static sf::Color themePlainFallback() {
    return sf::Color(255, 236, 236);
}

static sf::Color themeOverlayFill(bool bgLoaded) {
    return sf::Color(255, 245, 245, bgLoaded ? 130 : 245);
}

static void themeDrawBackdrop(sf::RenderWindow& window, sf::Sprite& bgSprite, bool bgLoaded, sf::RectangleShape& overlay) {
    window.clear(themePlainFallback());
    if (bgLoaded) window.draw(bgSprite);
    window.draw(overlay);
}

static void themeCenterTextX(sf::Text& t, int winW, float y) {
    sf::FloatRect b = t.getLocalBounds();
    t.setPosition((winW - b.width) * 0.5f - b.left, y);
}

/* Scrollable list: wheel to scroll; hover highlights; click or Enter confirms; Esc = cancel. Returns 1..count or 0. */
static int runVisualScrollPicker(const char* windowTitle, const char* headerLine, const char* const* lines, int count) {
    if (count <= 0) return 0;
    sf::RenderWindow window(sf::VideoMode(APP_WIN_W, APP_WIN_H), windowTitle);
    window.setFramerateLimit(60);

    sf::Font font;
    if (!loadUIFont(font)) return 0;

    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    bool bgLoaded = loadThemeTexture(bgTexture, THEME_BG_PATHS, THEME_BG_COUNT);
    if (bgLoaded) themeApplyBackgroundCover(bgSprite, bgTexture, APP_WIN_W, APP_WIN_H);

    sf::RectangleShape overlay(sf::Vector2f((float)APP_WIN_W, (float)APP_WIN_H));
    overlay.setFillColor(themeOverlayFill(bgLoaded));

    sf::Text head(headerLine, font, 22);
    head.setFillColor(sf::Color(35, 35, 35));
    themeCenterTextX(head, APP_WIN_W, 100.0f);

    sf::Text help("", font, 15);
    help.setFillColor(sf::Color(70, 70, 70));
    if (count > 9)
        help.setString("Scroll: wheel  |  Up/Down  |  click row  |  Enter = pick  |  type item # + Enter (e.g. 10)  |  Esc = cancel");
    else
        help.setString("Scroll: wheel  |  Up/Down then Enter  |  click row  |  Esc cancel  |  keys 1-9 quick pick");
    themeCenterTextX(help, APP_WIN_W, 134.0f);

    const float listPadX = 120.0f;
    const float listTop = 168.0f;
    const float listW = (float)APP_WIN_W - 2.0f * listPadX;
    const float listH = (float)APP_WIN_H - listTop - 56.0f;
    const float rowH = 36.0f;
    sf::RectangleShape listFrame(sf::Vector2f(listW, listH));
    listFrame.setPosition(listPadX, listTop);
    listFrame.setFillColor(sf::Color(255, 255, 255, 210));
    listFrame.setOutlineColor(sf::Color(70, 130, 180));
    listFrame.setOutlineThickness(2.0f);

    int nRows = (int)(listH / rowH); if (nRows < 1) nRows = 1;

    int scrollTop = 0;
    int highlight = 0;

    char pickBuf[12];
    pickBuf[0] = '\0';
    int pickLen = 0;
    int maxPickDigits = 1;
    for (int t = count; t >= 10; t /= 10) maxPickDigits++;

    sf::RectangleShape rowRect(sf::Vector2f(listW - 8.0f, rowH - 4.0f));
    sf::Text rowTxt("", font, 18);
    rowTxt.setFillColor(sf::Color(20, 35, 60));

    while (window.isOpen()) {
        sf::Vector2i mouse = sf::Mouse::getPosition(window);
        sf::Vector2f mousef((float)mouse.x, (float)mouse.y);

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) return 0;
            if (event.type == sf::Event::MouseWheelScrolled && event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                scrollTop -= (int)event.mouseWheelScroll.delta;
                if (scrollTop < 0) scrollTop = 0;
                int maxTop = count - nRows;
                if (maxTop < 0) maxTop = 0;
                if (scrollTop > maxTop) scrollTop = maxTop;
                /* Keep keyboard highlight visible after wheel */
                int lastVisible = scrollTop + nRows - 1;
                if (lastVisible >= count) lastVisible = count - 1;
                if (highlight < scrollTop) highlight = scrollTop;
                if (highlight > lastVisible) highlight = lastVisible;
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                float mx = (float)event.mouseButton.x;
                float my = (float)event.mouseButton.y;
                float rx = mx - listPadX;
                float ry = my - listTop;
                if (rx >= 0 && rx <= listW && ry >= 0 && ry <= listH) {
                    int ri = scrollTop + (int)(ry / rowH);
                    if (ri >= 0 && ri < count) {
                        pickLen = 0;
                        pickBuf[0] = '\0';
                        return ri + 1;
                    }
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) return 0;
                if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Enter) {
                    if (count > 9 && pickLen > 0) {
                        int picked = toInt(pickBuf);
                        pickLen = 0;
                        pickBuf[0] = '\0';
                        if (picked >= 1 && picked <= count) {
                            highlight = picked - 1;
                            int maxTop = count - nRows;
                            if (maxTop < 0) maxTop = 0;
                            if (highlight < scrollTop) scrollTop = highlight;
                            if (highlight >= scrollTop + nRows) scrollTop = highlight - nRows + 1;
                            if (scrollTop > maxTop) scrollTop = maxTop;
                            if (scrollTop < 0) scrollTop = 0;
                            return picked;
                        }
                    }
                    else
                        return highlight + 1;
                }
                if (count > 9) {
                    if (event.key.code == sf::Keyboard::Backspace) {
                        if (pickLen > 0) {
                            pickLen--;
                            pickBuf[pickLen] = '\0';
                        }
                    }
                    else {
                        int digit = -1;
                        if (event.key.code >= sf::Keyboard::Num0 && event.key.code <= sf::Keyboard::Num9)
                            digit = (int)(event.key.code - sf::Keyboard::Num0);
                        else if (event.key.code >= sf::Keyboard::Numpad0 && event.key.code <= sf::Keyboard::Numpad9)
                            digit = (int)(event.key.code - sf::Keyboard::Numpad0);
                        if (digit >= 0 && pickLen < maxPickDigits) {
                            pickBuf[pickLen++] = (char)('0' + digit);
                            pickBuf[pickLen] = '\0';
                        }
                    }
                }
                if (event.key.code == sf::Keyboard::Up) {
                    if (count > 9) {
                        pickLen = 0;
                        pickBuf[0] = '\0';
                    }
                    if (highlight > 0) highlight--;
                    if (highlight < scrollTop) scrollTop = highlight;
                }
                if (event.key.code == sf::Keyboard::Down) {
                    if (count > 9) {
                        pickLen = 0;
                        pickBuf[0] = '\0';
                    }
                    if (highlight < count - 1) highlight++;
                    if (highlight >= scrollTop + nRows) scrollTop = highlight - nRows + 1;
                }
                int maxTop = count - nRows;
                if (maxTop < 0) maxTop = 0;
                if (scrollTop > maxTop) scrollTop = maxTop;
                if (scrollTop < 0) scrollTop = 0;

                if (count <= 9 && event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num9) {
                    int d = (event.key.code - sf::Keyboard::Num1) + 1;
                    if (d <= count) return d;
                }
                if (count <= 9 && event.key.code >= sf::Keyboard::Numpad1 && event.key.code <= sf::Keyboard::Numpad9) {
                    int d = (event.key.code - sf::Keyboard::Numpad1) + 1;
                    if (d <= count) return d;
                }
            }
        }

        int maxTop = count - nRows;
        if (maxTop < 0) maxTop = 0;
        if (scrollTop > maxTop) scrollTop = maxTop;

        /* Hover row */
        int hoverRowIdx = -1;
        float rx = mousef.x - listPadX;
        float ry = mousef.y - listTop;
        if (rx >= 0 && rx <= listW && ry >= 0 && ry <= listH) {
            int hr = scrollTop + (int)(ry / rowH);
            if (hr >= 0 && hr < count)
                hoverRowIdx = hr;
        }

        themeDrawBackdrop(window, bgSprite, bgLoaded, overlay);
        themeCenterTextX(head, APP_WIN_W, 100.0f);
        window.draw(head);
        themeCenterTextX(help, APP_WIN_W, 134.0f);
        window.draw(help);
        window.draw(listFrame);

        for (int vis = 0; vis < nRows; vis++) {
            int i = scrollTop + vis;
            if (i >= count) break;

            rowRect.setPosition(listPadX + 4.0f, listTop + 6.0f + vis * rowH);

            sf::Color fill(245, 250, 255);
            if (hoverRowIdx == i) fill = sf::Color(212, 234, 255);
            else if (highlight == i) fill = sf::Color(198, 222, 255);
            rowRect.setFillColor(fill);
            rowRect.setOutlineThickness(1.5f);
            rowRect.setOutlineColor((highlight == i) ? sf::Color(50, 100, 150) : sf::Color(180, 200, 220));
            window.draw(rowRect);

            rowTxt.setString(lines[i]);
            rowTxt.setPosition(listPadX + 16.0f, listTop + 10.0f + vis * rowH);
            window.draw(rowTxt);
        }

        DynBuf fb;
        dynBufInit(&fb);
        dynBufAppendStr(&fb, "Showing ");
        dynBufAppendInt(&fb, scrollTop + 1);
        dynBufAppendStr(&fb, "-");
        int hiRow = scrollTop + nRows - 1;
        if (hiRow >= count) hiRow = count - 1;
        dynBufAppendInt(&fb, hiRow + 1);
        dynBufAppendStr(&fb, " of ");
        dynBufAppendInt(&fb, count);
        sf::Text foot(dynBufCStr(&fb), font, 15);
        dynBufFree(&fb);
        foot.setFillColor(sf::Color(80, 80, 80));
        themeCenterTextX(foot, APP_WIN_W, (float)APP_WIN_H - 36.0f);
        window.draw(foot);
        if (count > 9 && pickLen > 0) {
            DynBuf tb;
            dynBufInit(&tb);
            dynBufAppendStr(&tb, "Typed #: ");
            dynBufAppendStr(&tb, pickBuf);
            sf::Text typedFoot(dynBufCStr(&tb), font, 15);
            dynBufFree(&tb);
            typedFoot.setFillColor(sf::Color(30, 80, 130));
            themeCenterTextX(typedFoot, APP_WIN_W, (float)APP_WIN_H - 58.0f);
            window.draw(typedFoot);
        }
        window.display();
    }
    return 0;
}

bool getVisualTextInput(const char* title, const char* prompt, char* output, int maxLen, bool hideText = false) {
    sf::RenderWindow window(sf::VideoMode(APP_WIN_W, APP_WIN_H), title);
    window.setFramerateLimit(60);
    sf::Font font;
    if (!loadUIFont(font)) return false;

    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    bool bgLoaded = loadThemeTexture(bgTexture, THEME_BG_PATHS, THEME_BG_COUNT);
    if (bgLoaded) themeApplyBackgroundCover(bgSprite, bgTexture, APP_WIN_W, APP_WIN_H);

    sf::String typedInput;
    bool done = false;
    sf::Text promptText(prompt, font, 34);
    promptText.setFillColor(sf::Color(45, 45, 45));
    const float boxW = 680.0f;
    const float boxH = 62.0f;
    themeCenterTextX(promptText, APP_WIN_W, 278.0f);

    sf::RectangleShape inputBox(sf::Vector2f(boxW, boxH));
    inputBox.setPosition((APP_WIN_W - boxW) * 0.5f, 335.0f);
    inputBox.setFillColor(sf::Color(245, 250, 255));
    inputBox.setOutlineColor(sf::Color(70, 130, 180));
    inputBox.setOutlineThickness(2.0f);
    sf::Text inputText("", font, 32);
    inputText.setFillColor(sf::Color(25, 45, 70));
    inputText.setPosition((APP_WIN_W - boxW) * 0.5f + 18.0f, 348.0f);
    sf::Text hint("Enter = continue, Esc = cancel", font, 20);
    hint.setFillColor(sf::Color(70, 70, 70));
    themeCenterTextX(hint, APP_WIN_W, 422.0f);
    sf::RectangleShape overlay(sf::Vector2f((float)APP_WIN_W, (float)APP_WIN_H));
    overlay.setFillColor(themeOverlayFill(bgLoaded));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return false;
            }
            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == 8) {
                    if (!typedInput.isEmpty()) typedInput.erase(typedInput.getSize() - 1, 1);
                }
                else if (event.text.unicode >= 32 && event.text.unicode <= 126) {
                    if ((int)typedInput.getSize() < maxLen - 1) typedInput += event.text.unicode;
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter && !typedInput.isEmpty()) {
                    done = true;
                    window.close();
                }
                else if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                    return false;
                }
            }
        }

        sf::String shown = typedInput;
        if (hideText) {
            shown.clear();
            for (unsigned int i = 0; i < typedInput.getSize(); i++) shown += '*';
        }
        inputText.setString(shown);

        themeDrawBackdrop(window, bgSprite, bgLoaded, overlay);
        themeCenterTextX(promptText, APP_WIN_W, 278.0f);
        window.draw(promptText);
        window.draw(inputBox);
        window.draw(inputText);
        themeCenterTextX(hint, APP_WIN_W, 422.0f);
        window.draw(hint);
        window.display();
    }

    if (!done) return false;
    sfStringToCharArray(typedInput, output, maxLen);
    return true;
}

bool getVisualCredentials(int& id, char* password, int passMaxLen) {
    char idText[20];
    if (!getVisualTextInput("MediCore Login", "Enter ID", idText, 20, false)) return false;
    id = toInt(idText);
    if (id <= 0) return false;
    if (!getVisualTextInput("MediCore Login", "Enter Password", password, passMaxLen, true)) return false;
    return true;
}

bool loadThemeTexture(sf::Texture& texture, const char* paths[], int count) {
    for (int i = 0; i < count; i++) {
        if (texture.loadFromFile(paths[i])) return true;
    }
    return false;
}

int runVisualRoleMenu(const char* windowTitle, const char* header, const char* subtitle, const char* labels[], int optionCount, bool escSelectsLastMenuItem = true) {
    sf::RenderWindow window(sf::VideoMode(APP_WIN_W, APP_WIN_H), windowTitle);
    window.setFramerateLimit(60);

    sf::Font font;
    if (!loadUIFont(font)) return -1;

    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    bool bgLoaded = loadThemeTexture(bgTexture, THEME_BG_PATHS, THEME_BG_COUNT);
    if (bgLoaded) themeApplyBackgroundCover(bgSprite, bgTexture, APP_WIN_W, APP_WIN_H);

    sf::RectangleShape overlay(sf::Vector2f((float)APP_WIN_W, (float)APP_WIN_H));
    overlay.setFillColor(themeOverlayFill(bgLoaded));

    sf::Text title(header, font, 40);
    title.setFillColor(sf::Color(35, 35, 35));
    themeCenterTextX(title, APP_WIN_W, 118.0f);

    sf::Text sub(subtitle, font, 22);
    sub.setFillColor(sf::Color(55, 55, 55));
    themeCenterTextX(sub, APP_WIN_W, 168.0f);

    const int maxButtons = 15;
    sf::RectangleShape buttons[maxButtons];
    sf::Text texts[maxButtons];
    int shown = (optionCount > maxButtons) ? maxButtons : optionCount;
    const float btnW = 540.0f;
    float btnH = 40.0f;
    float rowGap = 6.0f;
    float firstY = 218.0f;
    float perRow = btnH + rowGap;
    float reservedBottom = 48.0f;
    float maxStack = (float)APP_WIN_H - firstY - reservedBottom;
    while ((float)shown * perRow > maxStack && btnH >= 30.0f) {
        btnH -= 2.0f;
        rowGap = 5.0f;
        perRow = btnH + rowGap;
    }
    float startX = ((float)APP_WIN_W - btnW) * 0.5f;
    for (int i = 0; i < shown; i++) {
        float y = firstY + i * (btnH + rowGap);
        buttons[i].setSize(sf::Vector2f(btnW, btnH));
        buttons[i].setPosition(startX, y);
        buttons[i].setFillColor(sf::Color(232, 245, 255, 235));
        buttons[i].setOutlineColor(sf::Color(60, 110, 155));
        buttons[i].setOutlineThickness(1.8f);

        texts[i].setFont(font);
        texts[i].setCharacterSize(20);
        texts[i].setString(labels[i]);
        texts[i].setFillColor(sf::Color(20, 40, 65));
        texts[i].setPosition(startX + 18.0f, y + 9.0f);
    }

    char numBuf[12];
    numBuf[0] = '\0';
    int numLen = 0;
    int maxMenuDigits = 1;
    for (int t = optionCount; t >= 10; t /= 10) maxMenuDigits++;

    sf::Text hint("", font, 18);
    hint.setFillColor(sf::Color(50, 50, 50));
    if (optionCount >= 10)
        hint.setString("Click an option, or press digits (0-9) then Enter (e.g. 1 0 Enter for 10), Backspace edits, Esc = logout");
    else
        hint.setString("Click an option, press number keys, or Esc to logout");
    themeCenterTextX(hint, APP_WIN_W, (float)APP_WIN_H - 42.0f);

    sf::Text typedNum("", font, 18);
    typedNum.setFillColor(sf::Color(30, 80, 130));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                return escSelectsLastMenuItem ? optionCount : 0;
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape)
                    return escSelectsLastMenuItem ? optionCount : 0;
                if (optionCount >= 10) {
                    if (event.key.code == sf::Keyboard::Backspace) {
                        if (numLen > 0) {
                            numLen--;
                            numBuf[numLen] = '\0';
                        }
                    }
                    else if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Enter) {
                        if (numLen > 0) {
                            int picked = toInt(numBuf);
                            numLen = 0;
                            numBuf[0] = '\0';
                            if (picked >= 1 && picked <= optionCount) return picked;
                        }
                    }
                    else {
                        int digit = -1;
                        if (event.key.code >= sf::Keyboard::Num0 && event.key.code <= sf::Keyboard::Num9)
                            digit = (int)(event.key.code - sf::Keyboard::Num0);
                        else if (event.key.code >= sf::Keyboard::Numpad0 && event.key.code <= sf::Keyboard::Numpad9)
                            digit = (int)(event.key.code - sf::Keyboard::Numpad0);
                        if (digit >= 0 && numLen < maxMenuDigits) {
                            numBuf[numLen++] = (char)('0' + digit);
                            numBuf[numLen] = '\0';
                        }
                    }
                }
                else {
                    if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num9) {
                        int picked = (event.key.code - sf::Keyboard::Num1) + 1;
                        if (picked >= 1 && picked <= optionCount) return picked;
                    }
                    if (event.key.code >= sf::Keyboard::Numpad1 && event.key.code <= sf::Keyboard::Numpad9) {
                        int picked = (event.key.code - sf::Keyboard::Numpad1) + 1;
                        if (picked >= 1 && picked <= optionCount) return picked;
                    }
                }
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos((float)event.mouseButton.x, (float)event.mouseButton.y);
                for (int i = 0; i < shown; i++) {
                    if (buttons[i].getGlobalBounds().contains(mousePos)) {
                        numLen = 0;
                        numBuf[0] = '\0';
                        return i + 1;
                    }
                }
            }
        }

        sf::Vector2i mouse = sf::Mouse::getPosition(window);
        for (int i = 0; i < shown; i++) {
            if (buttons[i].getGlobalBounds().contains((float)mouse.x, (float)mouse.y)) {
                buttons[i].setFillColor(sf::Color(205, 233, 255, 240));
            }
            else {
                buttons[i].setFillColor(sf::Color(232, 245, 255, 235));
            }
        }

        themeDrawBackdrop(window, bgSprite, bgLoaded, overlay);
        themeCenterTextX(title, APP_WIN_W, 118.0f);
        window.draw(title);
        themeCenterTextX(sub, APP_WIN_W, 168.0f);
        window.draw(sub);
        for (int i = 0; i < shown; i++) {
            window.draw(buttons[i]);
            window.draw(texts[i]);
        }
        themeCenterTextX(hint, APP_WIN_W, (float)APP_WIN_H - 42.0f);
        window.draw(hint);
        if (optionCount >= 10 && numLen > 0) {
            DynBuf tb;
            dynBufInit(&tb);
            dynBufAppendStr(&tb, "Typed: ");
            dynBufAppendStr(&tb, numBuf);
            typedNum.setString(dynBufCStr(&tb));
            dynBufFree(&tb);
            themeCenterTextX(typedNum, APP_WIN_W, (float)APP_WIN_H - 72.0f);
            window.draw(typedNum);
        }
        window.display();
    }
    return optionCount;
}

static int countContentLines(const char* s) {
    if (!s || !*s) return 1;
    int n = 1;
    for (const char* p = s; *p; p++) {
        if (*p == '\n') n++;
    }
    return n;
}

/* Pixel width of the first byteLen bytes of s (for wrapping long file lines inside the panel). */
static int measureTextPixels(const sf::Font& font, unsigned charSize, const char* s, int byteLen) {
    if (!s || byteLen <= 0) return 0;
    char tmp[2048];
    if (byteLen >= (int)sizeof(tmp)) byteLen = (int)sizeof(tmp) - 1;
    memcpy(tmp, s, byteLen);
    tmp[byteLen] = '\0';
    sf::Text m;
    m.setFont(font);
    m.setCharacterSize(charSize);
    m.setString(tmp);
    return (int)m.getLocalBounds().width;
}

/* Rewrap content so each line fits maxLinePixels; result is used for vertical scrolling only. */
static void appendWrappedContent(DynBuf* out, const sf::Font& font, unsigned charSize, const char* content, float maxLinePixels) {
    if (!content || content[0] == '\0') {
        dynBufAppendStr(out, "\n");
        return;
    }
    const char* seg = content;
    while (*seg) {
        const char* lineEnd = seg;
        while (*lineEnd && *lineEnd != '\n' && *lineEnd != '\r') lineEnd++;
        int lineLen = (int)(lineEnd - seg);
        if (lineLen == 0) {
            dynBufAppendStr(out, "\n");
        }
        else {
            int pos = 0;
            while (pos < lineLen) {
                int lo = pos + 1, hi = lineLen, best = pos;
                while (lo <= hi) {
                    int mid = (lo + hi) / 2;
                    int w = measureTextPixels(font, charSize, seg + pos, mid - pos);
                    if (w <= maxLinePixels) {
                        best = mid;
                        lo = mid + 1;
                    }
                    else hi = mid - 1;
                }
                if (best <= pos) best = pos + 1;
                int cut = best;
                if (cut < lineLen && seg[cut] != ' ') {
                    int sp = cut - 1;
                    while (sp > pos && seg[sp] != ' ') sp--;
                    if (sp > pos) cut = sp;
                }
                if (cut <= pos) cut = pos + 1;
                char piece[2048];
                int plen = cut - pos;
                if (plen >= (int)sizeof(piece)) plen = (int)sizeof(piece) - 1;
                memcpy(piece, seg + pos, plen);
                piece[plen] = '\0';
                dynBufAppendStr(out, piece);
                dynBufAppendStr(out, "\n");
                pos = cut;
                while (pos < lineLen && seg[pos] == ' ') pos++;
            }
        }
        if (*lineEnd == '\r') lineEnd++;
        if (*lineEnd == '\n') lineEnd++;
        seg = lineEnd;
    }
}

void showVisualTextWindow(const char* title, const char* header, const char* content) {
    sf::RenderWindow window(sf::VideoMode(APP_WIN_W, APP_WIN_H), title);
    window.setFramerateLimit(60);
    sf::Font font;
    if (!loadUIFont(font)) return;

    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    bool bgLoaded = loadThemeTexture(bgTexture, THEME_BG_PATHS, THEME_BG_COUNT);
    if (bgLoaded) themeApplyBackgroundCover(bgSprite, bgTexture, APP_WIN_W, APP_WIN_H);

    sf::RectangleShape overlay(sf::Vector2f((float)APP_WIN_W, (float)APP_WIN_H));
    overlay.setFillColor(themeOverlayFill(bgLoaded));

    const float panelW = 880.0f;
    const float panelH = 540.0f;
    float panelX = ((float)APP_WIN_W - panelW) * 0.5f;
    float panelY = ((float)APP_WIN_H - panelH) * 0.5f;
    sf::RectangleShape panel(sf::Vector2f(panelW, panelH));
    panel.setPosition(panelX, panelY);
    panel.setFillColor(sf::Color(255, 255, 255, 236));
    panel.setOutlineColor(sf::Color(70, 130, 180));
    panel.setOutlineThickness(2.0f);

    sf::Text titleText(header, font, 30);
    titleText.setFillColor(sf::Color(35, 35, 35));
    themeCenterTextX(titleText, APP_WIN_W, panelY + 14.0f);

    DynBuf wrapped;
    dynBufInit(&wrapped);
    appendWrappedContent(&wrapped, font, 19, content ? content : "", panelW - 48.0f);
    const char* textPtr = dynBufCStr(&wrapped);

    const float bodyTop = panelY + 56.0f;
    const float bodyBottom = panelY + panelH - 52.0f;
    const float lineStep = 23.0f;
    int scrollTopLine = 0;
    int totalLines = countContentLines(textPtr);
    int visibleLines = (int)((bodyBottom - bodyTop) / lineStep);
    if (visibleLines < 1) visibleLines = 1;
    int maxScrollTop = totalLines - visibleLines;
    if (maxScrollTop < 0) maxScrollTop = 0;

    sf::Text bodyLine("", font, 19);
    bodyLine.setFillColor(sf::Color(40, 40, 40));

    sf::Text hint("", font, 16);
    hint.setFillColor(sf::Color(60, 60, 60));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                dynBufFree(&wrapped);
                return;
            }
            if (event.type == sf::Event::MouseWheelScrolled && event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                scrollTopLine -= (int)event.mouseWheelScroll.delta;
                if (scrollTopLine < 0) scrollTopLine = 0;
                if (scrollTopLine > maxScrollTop) scrollTopLine = maxScrollTop;
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Escape) {
                    dynBufFree(&wrapped);
                    return;
                }
                if (event.key.code == sf::Keyboard::Up) {
                    if (scrollTopLine > 0) scrollTopLine--;
                }
                if (event.key.code == sf::Keyboard::Down) {
                    if (scrollTopLine < maxScrollTop) scrollTopLine++;
                }
                if (event.key.code == sf::Keyboard::PageUp) {
                    scrollTopLine -= visibleLines;
                    if (scrollTopLine < 0) scrollTopLine = 0;
                }
                if (event.key.code == sf::Keyboard::PageDown) {
                    scrollTopLine += visibleLines;
                    if (scrollTopLine > maxScrollTop) scrollTopLine = maxScrollTop;
                }
            }
        }
        themeDrawBackdrop(window, bgSprite, bgLoaded, overlay);
        window.draw(panel);
        themeCenterTextX(titleText, APP_WIN_W, panelY + 14.0f);
        window.draw(titleText);

        /* Draw body: skip to first visible line, print until past bottom of panel */
        const char* p = textPtr;
        int lineIdx = 0;
        while (*p && lineIdx < scrollTopLine) {
            if (*p == '\n') lineIdx++;
            p++;
        }
        float y = bodyTop;
        char lineBuf[2048];
        while (*p && y + lineStep <= bodyBottom) {
            const char* lineStart = p;
            while (*p && *p != '\n' && *p != '\r') p++;
            int len = (int)(p - lineStart);
            if (len >= (int)sizeof(lineBuf)) len = (int)sizeof(lineBuf) - 1;
            if (len > 0) memcpy(lineBuf, lineStart, len);
            lineBuf[len] = '\0';
            bodyLine.setString(lineBuf);
            bodyLine.setPosition(panelX + 24.0f, y);
            window.draw(bodyLine);
            y += lineStep;
            if (*p == '\r') p++;
            if (*p == '\n') p++;
        }

        if (maxScrollTop > 0) {
            DynBuf hb;
            dynBufInit(&hb);
            dynBufAppendStr(&hb, "Scroll: mouse wheel or Up/Down (PgUp/PgDn)  |  lines ");
            dynBufAppendInt(&hb, scrollTopLine + 1);
            dynBufAppendStr(&hb, "-");
            int lastShown = scrollTopLine + visibleLines;
            if (lastShown > totalLines) lastShown = totalLines;
            dynBufAppendInt(&hb, lastShown);
            dynBufAppendStr(&hb, " of ");
            dynBufAppendInt(&hb, totalLines);
            dynBufAppendStr(&hb, "  |  Enter/Esc close");
            hint.setString(dynBufCStr(&hb));
            dynBufFree(&hb);
        }
        else {
            hint.setString("Press Enter or Esc to close");
        }
        themeCenterTextX(hint, APP_WIN_W, panelY + panelH - 36.0f);
        window.draw(hint);
        window.display();
    }
}

void showVisualMessage(const char* title, const char* message) {
    showVisualTextWindow(title, "Message", message);
}

bool getVisualIntInput(const char* title, const char* prompt, int& out, bool positiveOnly = false) {
    char text[40];
    if (!getVisualTextInput(title, prompt, text, 40, false)) return false;
    out = toInt(text);
    if (positiveOnly && out <= 0) return false;
    return true;
}

bool getVisualFloatInput(const char* title, const char* prompt, float& out, bool positiveOnly = false) {
    char text[40];
    if (!getVisualTextInput(title, prompt, text, 40, false)) return false;
    out = (float)atof(text);
    if (positiveOnly && out <= 0.0f) return false;
    return true;
}

int getVisualMainMenuChoice() {
    sf::RenderWindow window(sf::VideoMode(APP_WIN_W, APP_WIN_H), "MediCore Hospital Management System");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!loadUIFont(font)) return 0;

    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    bool bgLoaded = loadThemeTexture(bgTexture, THEME_BG_PATHS, THEME_BG_COUNT);
    if (bgLoaded) themeApplyBackgroundCover(bgSprite, bgTexture, APP_WIN_W, APP_WIN_H);

    sf::Text title("Welcome to MediCore", font, 40);
    title.setFillColor(sf::Color(30, 30, 30));
    const float titleY = 210.0f;
    themeCenterTextX(title, APP_WIN_W, titleY);

    sf::Text subtitle("Hospital Management System", font, 26);
    subtitle.setFillColor(sf::Color(55, 55, 55));
    themeCenterTextX(subtitle, APP_WIN_W, titleY + 48.0f);

    sf::Text roleLine("Choose your login role", font, 20);
    roleLine.setFillColor(sf::Color(75, 75, 75));
    themeCenterTextX(roleLine, APP_WIN_W, titleY + 88.0f);

    const char* labels[5] = {
        "1. Patient Login",
        "2. Doctor Login",
        "3. Admin Login",
        "4. Register New Patient",
        "5. Exit"
    };

    const float btnW = 460.0f;
    const float btnH = 48.0f;
    const float rowGap = 10.0f;
    float firstBtnY = titleY + 138.0f;
    float startX = ((float)APP_WIN_W - btnW) * 0.5f;

    sf::RectangleShape buttons[5];
    sf::Text buttonTexts[5];
    for (int i = 0; i < 5; i++) {
        float y = firstBtnY + i * (btnH + rowGap);
        buttons[i].setSize(sf::Vector2f(btnW, btnH));
        buttons[i].setPosition(startX, y);
        buttons[i].setFillColor(sf::Color(230, 241, 255));
        buttons[i].setOutlineColor(sf::Color(70, 130, 180));
        buttons[i].setOutlineThickness(2.0f);

        buttonTexts[i].setFont(font);
        buttonTexts[i].setCharacterSize(22);
        buttonTexts[i].setString(labels[i]);
        buttonTexts[i].setFillColor(sf::Color(20, 40, 65));
        buttonTexts[i].setPosition(startX + 22.0f, y + 12.0f);
    }

    sf::Text hint("Click a button or press keys 1-5  |  Esc = Exit", font, 18);
    hint.setFillColor(sf::Color(40, 40, 40));
    themeCenterTextX(hint, APP_WIN_W, (float)APP_WIN_H - 40.0f);

    sf::RectangleShape overlay(sf::Vector2f((float)APP_WIN_W, (float)APP_WIN_H));
    overlay.setFillColor(themeOverlayFill(bgLoaded));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return 5;
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Num1 || event.key.code == sf::Keyboard::Numpad1) return 1;
                if (event.key.code == sf::Keyboard::Num2 || event.key.code == sf::Keyboard::Numpad2) return 2;
                if (event.key.code == sf::Keyboard::Num3 || event.key.code == sf::Keyboard::Numpad3) return 3;
                if (event.key.code == sf::Keyboard::Num4 || event.key.code == sf::Keyboard::Numpad4) return 4;
                if (event.key.code == sf::Keyboard::Num5 || event.key.code == sf::Keyboard::Numpad5 || event.key.code == sf::Keyboard::Escape) return 5;
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos((float)event.mouseButton.x, (float)event.mouseButton.y);
                for (int i = 0; i < 5; i++) {
                    if (buttons[i].getGlobalBounds().contains(mousePos)) return i + 1;
                }
            }
        }

        sf::Vector2i mouse = sf::Mouse::getPosition(window);
        for (int i = 0; i < 5; i++) {
            if (buttons[i].getGlobalBounds().contains((float)mouse.x, (float)mouse.y)) {
                buttons[i].setFillColor(sf::Color(210, 230, 255));
            }
            else {
                buttons[i].setFillColor(sf::Color(230, 241, 255));
            }
        }

        themeDrawBackdrop(window, bgSprite, bgLoaded, overlay);
        themeCenterTextX(title, APP_WIN_W, titleY);
        window.draw(title);
        themeCenterTextX(subtitle, APP_WIN_W, titleY + 48.0f);
        window.draw(subtitle);
        themeCenterTextX(roleLine, APP_WIN_W, titleY + 88.0f);
        window.draw(roleLine);
        for (int i = 0; i < 5; i++) {
            window.draw(buttons[i]);
            window.draw(buttonTexts[i]);
        }
        themeCenterTextX(hint, APP_WIN_W, (float)APP_WIN_H - 40.0f);
        window.draw(hint);
        window.display();
    }
    return 5;
}

// ==============LOGIN==============================

Patient* patientLogin(Storage<Patient>& patients) {
    int id;
    char password[50];
    if (!getVisualCredentials(id, password, 50)) return nullptr;

    Patient* all = patients.getAll();
    for (int i = 0; i < patients.size(); i++) {
        if (all[i].getId() == id) {
            if (stringCompare(password, all[i].getPassword())) {
                return &all[i];
            }
        }
    }
    return nullptr;
}

Doctor* doctorLogin(Storage<Doctor>& doctors) {
    int id;
    char password[50];
    if (!getVisualCredentials(id, password, 50)) return nullptr;

    Doctor* all = doctors.getAll();
    for (int i = 0; i < doctors.size(); i++) {
        if (all[i].getId() == id) {
            if (stringCompare(password, all[i].getPassword())) {
                return &all[i];
            }
        }
    }
    return nullptr;
}

bool adminLogin(Admin& admin) {
    int id;
    char password[50];
    if (!getVisualCredentials(id, password, 50)) return false;

    if (admin.getId() == id && stringCompare(password, admin.getPassword())) {
        return true;
    }
    return false;
}

// ==================== REGISTER ====================

void registerPatient(Storage<Patient>& patients) {
    if (!registerPatientVisual(patients)) showVisualMessage("Patient Registration", "Registration cancelled or invalid input.");
    else showVisualMessage("Patient Registration", "Registration successful.");
}

bool registerPatientVisual(Storage<Patient>& patients) {
    char name[100], ageText[10], genderText[5], contact[20], password[50];
    if (!getVisualTextInput("Patient Registration", "Enter Name", name, 100, false)) return false;
    if (!getVisualTextInput("Patient Registration", "Enter Age", ageText, 10, false)) return false;
    if (!getVisualTextInput("Patient Registration", "Enter Gender (M/F)", genderText, 5, false)) return false;
    if (!getVisualTextInput("Patient Registration", "Enter Contact (11 digits)", contact, 20, false)) return false;
    if (!getVisualTextInput("Patient Registration", "Create Password (min 6 chars)", password, 50, true)) return false;

    int age = toInt(ageText);
    char gender = genderText[0];
    if (gender >= 'a' && gender <= 'z') gender -= 32;
    if (gender != 'M' && gender != 'F') return false;
    if (!Validator::validateContact(contact)) return false;
    if (!Validator::validatePassword(password)) return false;
    if (age <= 0) return false;

    int newId = FileHandler::getNextUniqueId("patients.txt");
    Patient newPatient(name, newId, password, contact, age, gender, 0);
    patients.add(newPatient);
    FileHandler::appendPatient(newPatient);
    FileHandler::logSecurity("Patient", newId, "Registered");
    return true;
}

// ==================== PATIENT MENU ====================

void bookAppointment(Patient* p, Storage<Doctor>& doctors, Storage<Appointment>& appointments, Storage<Bill>& bills) {
    Doctor* allDoctors = doctors.getAll();
    char uniqueSpecs[100][100];
    int specCount = 0;
    for (int i = 0; i < doctors.size(); i++) {
        const char* spec = allDoctors[i].Getspecialization();
        bool exists = false;
        for (int j = 0; j < specCount; j++) {
            if (stringCompare(uniqueSpecs[j], spec)) {
                exists = true;
                break;
            }
        }
        if (!exists && specCount < 100) {
            int k = 0;
            while (spec[k] != '\0' && k < 99) {
                uniqueSpecs[specCount][k] = spec[k];
                k++;
            }
            uniqueSpecs[specCount][k] = '\0';
            specCount++;
        }
    }

    if (specCount == 0) { showVisualMessage("Book Appointment", "No specializations available right now."); return; }

    const char* specPtrs[100];
    for (int i = 0; i < specCount; i++) specPtrs[i] = uniqueSpecs[i];
    int specPick = runVisualScrollPicker("Book Appointment", "Choose specialization", specPtrs, specCount);
    if (specPick == 0) return;

    char specialization[100];
    int s = 0;
    while (uniqueSpecs[specPick - 1][s] != '\0' && s < 99) {
        specialization[s] = uniqueSpecs[specPick - 1][s];
        s++;
    }
    specialization[s] = '\0';

    int matchIds[100];
    char doctorRows[100][220];
    const char* dptrs[100];
    int matchCount = 0;
    for (int i = 0; i < doctors.size(); i++) {
        if (!stringCompare(allDoctors[i].Getspecialization(), specialization)) continue;
        matchIds[matchCount] = allDoctors[i].getId();
        DynBuf line;
        dynBufInit(&line);
        dynBufAppendStr(&line, "ID ");
        dynBufAppendInt(&line, allDoctors[i].getId());
        dynBufAppendStr(&line, " | ");
        dynBufAppendStr(&line, allDoctors[i].getName());
        dynBufAppendStr(&line, " | PKR ");
        dynBufAppendFloat2(&line, allDoctors[i].Getfee());
        stringCopy(doctorRows[matchCount], dynBufCStr(&line));
        dynBufFree(&line);
        dptrs[matchCount] = doctorRows[matchCount];
        matchCount++;
    }
    if (matchCount == 0) {
        showVisualMessage("Book Appointment", "No doctors available for that specialization.");
        return;
    }
    int docPick = runVisualScrollPicker("Book Appointment", "Choose doctor", dptrs, matchCount);
    if (docPick == 0) return;
    int doctorId = matchIds[docPick - 1];
    Doctor* selectedDoctor = nullptr;
    for (int i = 0; i < doctors.size(); i++) {
        if (allDoctors[i].getId() == doctorId) { selectedDoctor = &allDoctors[i]; break; }
    }
    if (selectedDoctor == nullptr) { showVisualMessage("Book Appointment", "Doctor not found."); return; }
    char date[20];
    int attempts = 0;
    while (attempts < 3) {
        if (!getVisualTextInput("Book Appointment", "Enter date (DD-MM-YYYY)", date, 20, false)) return;
        if (Validator::validateDate(date)) break;
        showVisualMessage("Book Appointment", "Invalid date. Use format DD-MM-YYYY.");
        attempts++;
    }
    if (attempts == 3) return;
    const char* slots[] = { "09:00","10:00","11:00","12:00","13:00","14:00","15:00","16:00" };
    bool available[8];
    for (int i = 0; i < 8; i++) available[i] = true;
    Appointment* allAppointments = appointments.getAll();
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getdocid() == doctorId && stringCompare(allAppointments[i].getdate(), date)) {
            if (!stringCompare(allAppointments[i].getstatus(), "cancelled")) {
                for (int k = 0; k < 8; k++) {
                    if (stringCompare(allAppointments[i].gettime(), slots[k]))
                        available[k] = false;
                }
            }
        }
    }
    char slotLineBufs[8][48];
    const char* slotPtrs[8];
    int availOrigIdx[8];
    int availN = 0;
    for (int si = 0; si < 8; si++) {
        if (available[si]) {
            stringCopy(slotLineBufs[availN], slots[si]);
            slotPtrs[availN] = slotLineBufs[availN];
            availOrigIdx[availN] = si;
            availN++;
        }
    }
    if (availN == 0) {
        showVisualMessage("Book Appointment", "No available time slots for this doctor on that date.");
        return;
    }
    int slotPick = runVisualScrollPicker("Book Appointment", "Choose an available time slot", slotPtrs, availN);
    if (slotPick == 0) return;
    int slotIndex = availOrigIdx[slotPick - 1];
    char timeSlot[10];
    stringCopy(timeSlot, slots[slotIndex]);
    if (p->getBalance() < selectedDoctor->Getfee()) {
        try { throw InsufficientFundsException(); }
        catch (InsufficientFundsException& e) { showVisualMessage("Book Appointment", e.what()); }
        return;
    }
    int newAppointmentId = FileHandler::getNextUniqueId("appointments.txt");
    Appointment newAppointment(newAppointmentId, p->getId(), doctorId, date, timeSlot, "pending");
    appointments.add(newAppointment);
    FileHandler::appendAppointment(newAppointment);
    *p -= selectedDoctor->Getfee();
    FileHandler::updatePatient(*p);
    int newBillId = FileHandler::getNextUniqueId("bills.txt");
    Bill newBill(newBillId, p->getId(), newAppointmentId, selectedDoctor->Getfee(), "unpaid", date);
    bills.add(newBill);
    FileHandler::appendBill(newBill);
    DynBuf ok;
    dynBufInit(&ok);
    dynBufAppendStr(&ok, "Appointment booked successfully.\nAppointment ID: ");
    dynBufAppendInt(&ok, newAppointmentId);
    showVisualTextWindow("Book Appointment", "Success", dynBufCStr(&ok));
    dynBufFree(&ok);
    FileHandler::logSecurity("Patient", p->getId(), "BookedAppointment");
}

void cancelAppointment(Patient* p, Storage<Doctor>& doctors, Storage<Appointment>& appointments, Storage<Bill>& bills) {
    Appointment* allAppointments = appointments.getAll();
    Doctor* allDoctors = doctors.getAll();
    int pendAppIds[100];
    char pendRows[100][360];
    const char* pptrs[100];
    int nPend = 0;
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getpatid() != p->getId() || !stringCompare(allAppointments[i].getstatus(), "pending"))
            continue;
        char doctorName[100] = "Unknown";
        for (int j = 0; j < doctors.size(); j++) {
            if (allDoctors[j].getId() == allAppointments[i].getdocid()) {
                int k = 0;
                while (allDoctors[j].getName()[k] != '\0') { doctorName[k] = allDoctors[j].getName()[k]; k++; }
                doctorName[k] = '\0';
                break;
            }
        }
        pendAppIds[nPend] = allAppointments[i].getappid();
        DynBuf ln;
        dynBufInit(&ln);
        dynBufAppendStr(&ln, "App ");
        dynBufAppendInt(&ln, allAppointments[i].getappid());
        dynBufAppendStr(&ln, " | ");
        dynBufAppendStr(&ln, doctorName);
        dynBufAppendStr(&ln, " | ");
        dynBufAppendStr(&ln, allAppointments[i].getdate());
        dynBufAppendStr(&ln, " ");
        dynBufAppendStr(&ln, allAppointments[i].gettime());
        stringCopy(pendRows[nPend], dynBufCStr(&ln));
        dynBufFree(&ln);
        pptrs[nPend] = pendRows[nPend];
        nPend++;
    }
    if (nPend == 0) { showVisualMessage("Cancel Appointment", "You have no pending appointments."); return; }
    int pick = runVisualScrollPicker("Cancel Appointment", "Tap or scroll to choose a pending appointment to cancel", pptrs, nPend);
    if (pick == 0) return;
    int appointmentId = pendAppIds[pick - 1];
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getappid() == appointmentId &&
            allAppointments[i].getpatid() == p->getId() &&
            stringCompare(allAppointments[i].getstatus(), "pending")) {
            Doctor* allDoctors = doctors.getAll();
            float fee = 0;
            for (int j = 0; j < doctors.size(); j++) {
                if (allDoctors[j].getId() == allAppointments[i].getdocid()) { fee = allDoctors[j].Getfee(); break; }
            }
            Appointment cancelled(appointmentId, allAppointments[i].getpatid(),
                allAppointments[i].getdocid(), allAppointments[i].getdate(),
                allAppointments[i].gettime(), "cancelled");
            allAppointments[i] = cancelled;
            FileHandler::updateAppointment(cancelled);
            *p += fee;
            FileHandler::updatePatient(*p);
            Bill* allBills = bills.getAll();
            for (int j = 0; j < bills.size(); j++) {
                if (allBills[j].getappid() == appointmentId) {
                    Bill cancelledBill(allBills[j].getbillid(), allBills[j].getpatid(),
                        allBills[j].getappid(), allBills[j].getamount(), "cancelled", allBills[j].getdate());
                    allBills[j] = cancelledBill;
                    FileHandler::updateBill(cancelledBill);
                    break;
                }
            }
            DynBuf msg;
            dynBufInit(&msg);
            dynBufAppendStr(&msg, "Appointment cancelled.\nPKR ");
            dynBufAppendFloat2(&msg, fee);
            dynBufAppendStr(&msg, " refunded to your balance.");
            showVisualTextWindow("Cancel Appointment", "Success", dynBufCStr(&msg));
            dynBufFree(&msg);
            FileHandler::logSecurity("Patient", p->getId(), "CancelledAppointment");
            return;
        }
    }
    showVisualMessage("Cancel Appointment", "Invalid appointment ID.");
}

void viewMyAppointments(Patient* p, Storage<Appointment>& appointments, Storage<Doctor>& doctors) {
    Appointment* allAppointments = appointments.getAll();
    int found = 0;
    DynBuf content;
    dynBufInit(&content);
    dynBufAppendStr(&content, "Appointment ID | Doctor Name | Date | Time Slot | Status\n");
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getpatid() == p->getId()) {
            Doctor* allDoctors = doctors.getAll();
            char doctorName[100] = "Unknown";
            for (int j = 0; j < doctors.size(); j++) {
                if (allDoctors[j].getId() == allAppointments[i].getdocid()) {
                    int k = 0;
                    while (allDoctors[j].getName()[k] != '\0') { doctorName[k] = allDoctors[j].getName()[k]; k++; }
                    doctorName[k] = '\0'; break;
                }
            }
            dynBufAppendInt(&content, allAppointments[i].getappid());
            dynBufAppendStr(&content, " | ");
            dynBufAppendStr(&content, doctorName);
            dynBufAppendStr(&content, " | ");
            dynBufAppendStr(&content, allAppointments[i].getdate());
            dynBufAppendStr(&content, " | ");
            dynBufAppendStr(&content, allAppointments[i].gettime());
            dynBufAppendStr(&content, " | ");
            dynBufAppendStr(&content, allAppointments[i].getstatus());
            dynBufAppendStr(&content, "\n");
            found++;
        }
    }
    if (found == 0) { showVisualMessage("My Appointments", "No appointments found."); dynBufFree(&content); }
    else { showVisualTextWindow("My Appointments", "Appointments", dynBufCStr(&content)); dynBufFree(&content); }
}

void viewMyMedicalRecords(Patient* p, Storage<Prescription>& prescriptions, Storage<Doctor>& doctors) {
    Prescription* allPrescriptions = prescriptions.getAll();
    int found = 0;
    DynBuf content;
    dynBufInit(&content);
    dynBufAppendStr(&content, "Date | Doctor Name | Medicines | Notes\n");
    for (int i = 0; i < prescriptions.size(); i++) {
        if (allPrescriptions[i].getpatid() == p->getId()) {
            Doctor* allDoctors = doctors.getAll();
            char doctorName[100] = "Unknown";
            for (int j = 0; j < doctors.size(); j++) {
                if (allDoctors[j].getId() == allPrescriptions[i].getdocid()) {
                    int k = 0;
                    while (allDoctors[j].getName()[k] != '\0') { doctorName[k] = allDoctors[j].getName()[k]; k++; }
                    doctorName[k] = '\0'; break;
                }
            }
            dynBufAppendStr(&content, "Date: ");
            dynBufAppendStr(&content, allPrescriptions[i].getdate());
            dynBufAppendStr(&content, " | Doctor: ");
            dynBufAppendStr(&content, doctorName);
            dynBufAppendStr(&content, " | Medicines: ");
            dynBufAppendStr(&content, allPrescriptions[i].getmed());
            dynBufAppendStr(&content, " | Notes: ");
            dynBufAppendStr(&content, allPrescriptions[i].getnotes());
            dynBufAppendStr(&content, "\n");
            found++;
        }
    }
    if (found == 0) { showVisualMessage("Medical Records", "No medical records found."); dynBufFree(&content); }
    else { showVisualTextWindow("Medical Records", "My Medical Records", dynBufCStr(&content)); dynBufFree(&content); }
}

void viewMyBills(Patient* p, Storage<Bill>& bills) {
    Bill* allBills = bills.getAll();
    float totalDue = 0;
    int found = 0;
    DynBuf content;
    dynBufInit(&content);
    dynBufAppendStr(&content, "Bill ID | Appointment ID | Amount (PKR) | Status | Date\n");
    for (int i = 0; i < bills.size(); i++) {
        if (allBills[i].getpatid() == p->getId()) {
            dynBufAppendInt(&content, allBills[i].getbillid());
            dynBufAppendStr(&content, " | ");
            dynBufAppendInt(&content, allBills[i].getappid());
            dynBufAppendStr(&content, " | PKR ");
            dynBufAppendFloat2(&content, allBills[i].getamount());
            dynBufAppendStr(&content, " | ");
            dynBufAppendStr(&content, allBills[i].getstatus());
            dynBufAppendStr(&content, " | ");
            dynBufAppendStr(&content, allBills[i].getdate());
            dynBufAppendStr(&content, "\n");
            if (stringCompare(allBills[i].getstatus(), "unpaid"))
                totalDue += allBills[i].getamount();
            found++;
        }
    }
    if (found == 0) { showVisualMessage("My Bills", "No bills found."); dynBufFree(&content); }
    else {
        dynBufAppendStr(&content, "\nTotal outstanding unpaid: PKR ");
        dynBufAppendFloat2(&content, totalDue);
        showVisualTextWindow("My Bills", "Billing Overview", dynBufCStr(&content));
        dynBufFree(&content);
    }
}

void payBill(Patient* p, Storage<Bill>& bills) {
    Bill* allBills = bills.getAll();
    int billIds[100];
    char billRows[100][140];
    const char* pbill[100];
    int nb = 0;
    for (int i = 0; i < bills.size(); i++) {
        if (allBills[i].getpatid() != p->getId() || !stringCompare(allBills[i].getstatus(), "unpaid")) continue;
        billIds[nb] = allBills[i].getbillid();
        DynBuf ln;
        dynBufInit(&ln);
        dynBufAppendStr(&ln, "Bill ");
        dynBufAppendInt(&ln, allBills[i].getbillid());
        dynBufAppendStr(&ln, " | PKR ");
        dynBufAppendFloat2(&ln, allBills[i].getamount());
        stringCopy(billRows[nb], dynBufCStr(&ln));
        dynBufFree(&ln);
        pbill[nb] = billRows[nb];
        nb++;
    }
    if (nb == 0) { showVisualMessage("Pay Bill", "No unpaid bills."); return; }
    int pick = runVisualScrollPicker("Pay Bill", "Choose unpaid bill to pay", pbill, nb);
    if (pick == 0) return;
    int billId = billIds[pick - 1];
    for (int i = 0; i < bills.size(); i++) {
        if (allBills[i].getbillid() == billId && allBills[i].getpatid() == p->getId() && stringCompare(allBills[i].getstatus(), "unpaid")) {
            if (p->getBalance() < allBills[i].getamount()) {
                try { throw InsufficientFundsException(); }
                catch (InsufficientFundsException& e) { showVisualMessage("Pay Bill", e.what()); }
                return;
            }
            *p -= allBills[i].getamount();
            Bill paidBill(allBills[i].getbillid(), allBills[i].getpatid(),
                allBills[i].getappid(), allBills[i].getamount(), "paid", allBills[i].getdate());
            allBills[i] = paidBill;
            FileHandler::updatePatient(*p);
            FileHandler::updateBill(paidBill);
            DynBuf msg;
            dynBufInit(&msg);
            dynBufAppendStr(&msg, "Bill paid successfully.\nRemaining balance: PKR ");
            dynBufAppendFloat2(&msg, p->getBalance());
            showVisualTextWindow("Pay Bill", "Success", dynBufCStr(&msg));
            dynBufFree(&msg);
            FileHandler::logSecurity("Patient", p->getId(), "PaidBill");
            return;
        }
    }
    showVisualMessage("Pay Bill", "Invalid bill ID.");
}

void topUpBalance(Patient* p) {
    float amount;
    int attempts = 0;
    while (attempts < 3) {
        if (!getVisualFloatInput("Top Up Balance", "Enter amount to add (PKR)", amount, true)) return;
        try {
            if (!Validator::validatePositiveFloat(amount)) throw InvalidInputException();
            *p += amount;
            FileHandler::updatePatient(*p);
            DynBuf msg;
            dynBufInit(&msg);
            dynBufAppendStr(&msg, "Balance updated.\nNew balance: PKR ");
            dynBufAppendFloat2(&msg, p->getBalance());
            showVisualTextWindow("Top Up Balance", "Success", dynBufCStr(&msg));
            dynBufFree(&msg);
            FileHandler::logSecurity("Patient", p->getId(), "ToppedUpBalance");
            return;
        }
        catch (InvalidInputException& e) { showVisualMessage("Top Up Balance", e.what()); attempts++; }
    }
}

void patientMenu(Patient* p, Storage<Doctor>& doctors, Storage<Appointment>& appointments, Storage<Bill>& bills, Storage<Prescription>& prescriptions) {
    int choice;
    const char* options[] = {
        "1. Book Appointment",
        "2. Cancel Appointment",
        "3. View My Appointments",
        "4. View My Medical Records",
        "5. View My Bills",
        "6. Pay Bill",
        "7. Top Up Balance",
        "8. Logout"
    };
    while (true) {
        DynBuf subtitle;
        dynBufInit(&subtitle);
        dynBufAppendStr(&subtitle, "Patient: ");
        dynBufAppendStr(&subtitle, p->getName());
        dynBufAppendStr(&subtitle, " | Balance: PKR ");
        dynBufAppendFloat2(&subtitle, p->getBalance());
        choice = runVisualRoleMenu("Patient Menu", "Patient Dashboard", dynBufCStr(&subtitle), options, 8);
        dynBufFree(&subtitle);
        if (choice <= 0) continue;
        if (choice == 1) bookAppointment(p, doctors, appointments, bills);
        else if (choice == 2) cancelAppointment(p, doctors, appointments, bills);
        else if (choice == 3) viewMyAppointments(p, appointments, doctors);
        else if (choice == 4) viewMyMedicalRecords(p, prescriptions, doctors);
        else if (choice == 5) viewMyBills(p, bills);
        else if (choice == 6) payBill(p, bills);
        else if (choice == 7) topUpBalance(p);
        else if (choice == 8) break;
        else showVisualMessage("Patient Menu", "Invalid option.");
    }
}

// ==================== DOCTOR MENU ====================

void viewTodaysAppointments(Doctor* d, Storage<Appointment>& appointments, Storage<Patient>& patients) {
    char today[11]; getCurrentDate(today);
    Appointment* allAppointments = appointments.getAll();
    int found = 0;
    DynBuf content;
    dynBufInit(&content);
    dynBufAppendStr(&content, "Appointment ID | Patient Name | Time Slot | Status\n");
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getdocid() == d->getId() && stringCompare(allAppointments[i].getdate(), today)) {
            Patient* allPatients = patients.getAll();
            char patientName[100] = "Unknown";
            for (int j = 0; j < patients.size(); j++) {
                if (allPatients[j].getId() == allAppointments[i].getpatid()) {
                    int k = 0;
                    while (allPatients[j].getName()[k] != '\0') { patientName[k] = allPatients[j].getName()[k]; k++; }
                    patientName[k] = '\0'; break;
                }
            }
            dynBufAppendInt(&content, allAppointments[i].getappid());
            dynBufAppendStr(&content, " | ");
            dynBufAppendStr(&content, patientName);
            dynBufAppendStr(&content, " | ");
            dynBufAppendStr(&content, allAppointments[i].gettime());
            dynBufAppendStr(&content, " | ");
            dynBufAppendStr(&content, allAppointments[i].getstatus());
            dynBufAppendStr(&content, "\n");
            found++;
        }
    }
    if (found == 0) { showVisualMessage("Today's Appointments", "No appointments scheduled for today."); dynBufFree(&content); }
    else { showVisualTextWindow("Today's Appointments", "Today's Appointments", dynBufCStr(&content)); dynBufFree(&content); }
}

void viewFutureAppointments(Doctor* d, Storage<Appointment>& appointments, Storage<Patient>& patients) {
    char today[11]; getCurrentDate(today);
    Appointment* allAppointments = appointments.getAll();
    int found = 0;
    DynBuf content;
    dynBufInit(&content);
    dynBufAppendStr(&content, "Appointment ID | Patient Name | Date | Time Slot | Status\n");
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getdocid() == d->getId()) {
            if (isDateAfter(allAppointments[i].getdate(), today)) {
                Patient* allPatients = patients.getAll();
                char patientName[100] = "Unknown";
                for (int j = 0; j < patients.size(); j++) {
                    if (allPatients[j].getId() == allAppointments[i].getpatid()) {
                        int k = 0;
                        while (allPatients[j].getName()[k] != '\0') { patientName[k] = allPatients[j].getName()[k]; k++; }
                        patientName[k] = '\0'; break;
                    }
                }
                dynBufAppendInt(&content, allAppointments[i].getappid());
                dynBufAppendStr(&content, " | ");
                dynBufAppendStr(&content, patientName);
                dynBufAppendStr(&content, " | ");
                dynBufAppendStr(&content, allAppointments[i].getdate());
                dynBufAppendStr(&content, " | ");
                dynBufAppendStr(&content, allAppointments[i].gettime());
                dynBufAppendStr(&content, " | ");
                dynBufAppendStr(&content, allAppointments[i].getstatus());
                dynBufAppendStr(&content, "\n");
                found++;
            }
        }
    }
    if (found == 0) { showVisualMessage("Future Appointments", "No future appointments found."); dynBufFree(&content); }
    else { showVisualTextWindow("Future Appointments", "Future Appointments", dynBufCStr(&content)); dynBufFree(&content); }
}

void markAppointmentComplete(Doctor* d, Storage<Appointment>& appointments) {
    char today[11]; getCurrentDate(today);
    Appointment* allAppointments = appointments.getAll();
    int candIds[100];
    char candRows[100][160];
    const char* cptrs[100];
    int nCand = 0;
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getdocid() != d->getId() ||
            !stringCompare(allAppointments[i].getdate(), today) ||
            !stringCompare(allAppointments[i].getstatus(), "pending"))
            continue;
        candIds[nCand] = allAppointments[i].getappid();
        DynBuf ln;
        dynBufInit(&ln);
        dynBufAppendStr(&ln, "App ");
        dynBufAppendInt(&ln, allAppointments[i].getappid());
        dynBufAppendStr(&ln, " | ");
        dynBufAppendStr(&ln, allAppointments[i].gettime());
        dynBufAppendStr(&ln, " | pending");
        stringCopy(candRows[nCand], dynBufCStr(&ln));
        dynBufFree(&ln);
        cptrs[nCand] = candRows[nCand];
        nCand++;
    }
    if (nCand == 0) { showVisualMessage("Mark Complete", "No pending appointments today."); return; }
    int pick = runVisualScrollPicker("Mark Complete", "Today's pending appointments — select one to mark completed", cptrs, nCand);
    if (pick == 0) return;
    int appointmentId = candIds[pick - 1];
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getappid() == appointmentId &&
            allAppointments[i].getdocid() == d->getId() &&
            stringCompare(allAppointments[i].getdate(), today) &&
            stringCompare(allAppointments[i].getstatus(), "pending")) {
            Appointment completed(appointmentId, allAppointments[i].getpatid(),
                allAppointments[i].getdocid(), allAppointments[i].getdate(),
                allAppointments[i].gettime(), "completed");
            allAppointments[i] = completed;
            FileHandler::updateAppointment(completed);
            showVisualMessage("Mark Complete", "Appointment marked as completed.");
            return;
        }
    }
    showVisualMessage("Mark Complete", "Invalid appointment ID.");
}

void markAppointmentNoShow(Doctor* d, Storage<Appointment>& appointments, Storage<Bill>& bills) {
    char today[11]; getCurrentDate(today);
    Appointment* allAppointments = appointments.getAll();
    int candIds[100];
    char candRows[100][160];
    const char* cptrs[100];
    int nCand = 0;
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getdocid() != d->getId() ||
            !stringCompare(allAppointments[i].getdate(), today) ||
            !stringCompare(allAppointments[i].getstatus(), "pending"))
            continue;
        candIds[nCand] = allAppointments[i].getappid();
        DynBuf ln;
        dynBufInit(&ln);
        dynBufAppendStr(&ln, "App ");
        dynBufAppendInt(&ln, allAppointments[i].getappid());
        dynBufAppendStr(&ln, " | ");
        dynBufAppendStr(&ln, allAppointments[i].gettime());
        dynBufAppendStr(&ln, " | pending");
        stringCopy(candRows[nCand], dynBufCStr(&ln));
        dynBufFree(&ln);
        cptrs[nCand] = candRows[nCand];
        nCand++;
    }
    if (nCand == 0) { showVisualMessage("Mark No-Show", "No pending appointments today."); return; }
    int pick = runVisualScrollPicker("Mark No-Show", "Select appointment to mark as no-show", cptrs, nCand);
    if (pick == 0) return;
    int appointmentId = candIds[pick - 1];
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getappid() == appointmentId &&
            allAppointments[i].getdocid() == d->getId() &&
            stringCompare(allAppointments[i].getdate(), today) &&
            stringCompare(allAppointments[i].getstatus(), "pending")) {
            Appointment noshow(appointmentId, allAppointments[i].getpatid(),
                allAppointments[i].getdocid(), allAppointments[i].getdate(),
                allAppointments[i].gettime(), "noshow");
            allAppointments[i] = noshow;
            FileHandler::updateAppointment(noshow);
            Bill* allBills = bills.getAll();
            for (int j = 0; j < bills.size(); j++) {
                if (allBills[j].getappid() == appointmentId) {
                    Bill cancelledBill(allBills[j].getbillid(), allBills[j].getpatid(),
                        allBills[j].getappid(), allBills[j].getamount(), "cancelled", allBills[j].getdate());
                    allBills[j] = cancelledBill;
                    FileHandler::updateBill(cancelledBill);
                    break;
                }
            }
            showVisualMessage("Mark No-Show", "Appointment marked as no-show.");
            return;
        }
    }
    showVisualMessage("Mark No-Show", "Invalid appointment ID.");
}

void writePrescription(Doctor* d, Storage<Appointment>& appointments, Storage<Prescription>& prescriptions, Storage<Patient>& patients) {
    Appointment* allAppointments = appointments.getAll();
    Prescription* allPrescriptions = prescriptions.getAll();
    Patient* allPatients = patients.getAll();
    int candAppIds[100];
    char candRows[100][320];
    const char* rptrs[100];
    int nCand = 0;
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getdocid() != d->getId() || !stringCompare(allAppointments[i].getstatus(), "completed"))
            continue;
        bool hasRx = false;
        for (int r = 0; r < prescriptions.size(); r++) {
            if (allPrescriptions[r].getappid() == allAppointments[i].getappid()) { hasRx = true; break; }
        }
        if (hasRx) continue;
        char patientName[100] = "Unknown";
        for (int pj = 0; pj < patients.size(); pj++) {
            if (allPatients[pj].getId() == allAppointments[i].getpatid()) {
                int k = 0;
                while (allPatients[pj].getName()[k] != '\0') { patientName[k] = allPatients[pj].getName()[k]; k++; }
                patientName[k] = '\0';
                break;
            }
        }
        candAppIds[nCand] = allAppointments[i].getappid();
        DynBuf ln;
        dynBufInit(&ln);
        dynBufAppendStr(&ln, "App ");
        dynBufAppendInt(&ln, allAppointments[i].getappid());
        dynBufAppendStr(&ln, " | Patient ");
        dynBufAppendStr(&ln, patientName);
        dynBufAppendStr(&ln, " | ");
        dynBufAppendStr(&ln, allAppointments[i].getdate());
        stringCopy(candRows[nCand], dynBufCStr(&ln));
        dynBufFree(&ln);
        rptrs[nCand] = candRows[nCand];
        nCand++;
    }
    if (nCand == 0) {
        showVisualMessage("Write Prescription", "No completed appointments without a prescription.");
        return;
    }
    int pick = runVisualScrollPicker("Write Prescription", "Choose completed appointment (no prescription yet)", rptrs, nCand);
    if (pick == 0) return;
    int appointmentId = candAppIds[pick - 1];
    int patientId = -1;
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getappid() == appointmentId &&
            allAppointments[i].getdocid() == d->getId() &&
            stringCompare(allAppointments[i].getstatus(), "completed")) {
            patientId = allAppointments[i].getpatid(); break;
        }
    }
    if (patientId == -1) { showVisualMessage("Write Prescription", "Invalid appointment or appointment not completed."); return; }
    char medicines[500], notes[300];
    if (!getVisualTextInput("Write Prescription", "Enter medicines (semicolon separated)", medicines, 500, false)) return;
    if (!getVisualTextInput("Write Prescription", "Enter notes", notes, 300, false)) return;
    char today[11]; getCurrentDate(today);
    int newPrescriptionId = FileHandler::getNextUniqueId("prescriptions.txt");
    Prescription newPrescription(newPrescriptionId, appointmentId, patientId, d->getId(), today, medicines, notes);
    prescriptions.add(newPrescription);
    FileHandler::appendPrescription(newPrescription);
    showVisualMessage("Write Prescription", "Prescription saved.");
}

void viewPatientHistory(Doctor* d, Storage<Appointment>& appointments, Storage<Prescription>& prescriptions) {
    int patientId; if (!getVisualIntInput("Patient History", "Enter Patient ID", patientId, true)) return;
    Appointment* allAppointments = appointments.getAll();
    bool hasAccess = false;
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getpatid() == patientId &&
            allAppointments[i].getdocid() == d->getId() &&
            stringCompare(allAppointments[i].getstatus(), "completed")) {
            hasAccess = true; break;
        }
    }
    if (!hasAccess) { showVisualMessage("Patient History", "Access denied. You can only view records of your own patients."); return; }
    Prescription* allPrescriptions = prescriptions.getAll();
    int found = 0;
    DynBuf content;
    dynBufInit(&content);
    dynBufAppendStr(&content, "Patient Medical History\n\n");
    for (int i = 0; i < prescriptions.size(); i++) {
        if (allPrescriptions[i].getpatid() == patientId && allPrescriptions[i].getdocid() == d->getId()) {
            dynBufAppendStr(&content, "Date: ");
            dynBufAppendStr(&content, allPrescriptions[i].getdate());
            dynBufAppendStr(&content, " | Medicines: ");
            dynBufAppendStr(&content, allPrescriptions[i].getmed());
            dynBufAppendStr(&content, " | Notes: ");
            dynBufAppendStr(&content, allPrescriptions[i].getnotes());
            dynBufAppendStr(&content, "\n");
            found++;
        }
    }
    if (found == 0) { showVisualMessage("Patient History", "No records found."); dynBufFree(&content); }
    else { showVisualTextWindow("Patient History", "History", dynBufCStr(&content)); dynBufFree(&content); }
}

void doctorMenu(Doctor* d, Storage<Patient>& patients, Storage<Appointment>& appointments, Storage<Bill>& bills, Storage<Prescription>& prescriptions) {
    int choice;
    const char* options[] = {
        "1. View Today's Appointments",
        "2. Mark Appointment Complete",
        "3. Mark Appointment No-Show",
        "4. Write Prescription",
        "5. View Patient Medical History",
        "6. View Future Appointments",
        "7. Logout"
    };
    while (true) {
        DynBuf subtitle;
        dynBufInit(&subtitle);
        dynBufAppendStr(&subtitle, "Dr. ");
        dynBufAppendStr(&subtitle, d->getName());
        dynBufAppendStr(&subtitle, " | Specialization: ");
        dynBufAppendStr(&subtitle, d->Getspecialization());
        choice = runVisualRoleMenu("Doctor Menu", "Doctor Dashboard", dynBufCStr(&subtitle), options, 7);
        dynBufFree(&subtitle);
        if (choice <= 0) continue;
        if (choice == 1) viewTodaysAppointments(d, appointments, patients);
        else if (choice == 2) markAppointmentComplete(d, appointments);
        else if (choice == 3) markAppointmentNoShow(d, appointments, bills);
        else if (choice == 4) writePrescription(d, appointments, prescriptions, patients);
        else if (choice == 5) viewPatientHistory(d, appointments, prescriptions);
        else if (choice == 6) viewFutureAppointments(d, appointments, patients);
        else if (choice == 7) break;
        else showVisualMessage("Doctor Menu", "Invalid option.");
    }
}

// ==================== ADMIN MENU ====================

void addDoctor(Storage<Doctor>& doctors) {
    char name[50], specialization[50], contact[20], password[50];
    float fee;
    if (!getVisualTextInput("Add Doctor", "Enter Name", name, 50, false)) return;
    if (!getVisualTextInput("Add Doctor", "Enter Specialization", specialization, 50, false)) return;
    if (!getVisualTextInput("Add Doctor", "Enter Contact (11 digits)", contact, 20, false)) return;
    if (!Validator::validateContact(contact)) { showVisualMessage("Add Doctor", "Invalid contact!"); return; }
    if (!getVisualTextInput("Add Doctor", "Enter Password (min 6 chars)", password, 50, true)) return;
    if (!Validator::validatePassword(password)) { showVisualMessage("Add Doctor", "Invalid password!"); return; }
    if (!getVisualFloatInput("Add Doctor", "Enter Consultation Fee", fee, true)) return;
    if (!Validator::validatePositiveFloat(fee)) { showVisualMessage("Add Doctor", "Invalid fee!"); return; }
    int newId = FileHandler::getNextUniqueId("doctors.txt");
    Doctor newDoctor(name, newId, password, specialization, contact, fee);
    doctors.add(newDoctor);
    FileHandler::appendDoctor(newDoctor);
    DynBuf msg;
    dynBufInit(&msg);
    dynBufAppendStr(&msg, "Doctor added successfully.\nID: ");
    dynBufAppendInt(&msg, newId);
    showVisualTextWindow("Add Doctor", "Success", dynBufCStr(&msg));
    dynBufFree(&msg);
}

void unlockPatientAccount(Storage<Patient>& patients) {
    const char* howLabels[] = {
        "1. Type Patient ID",
        "2. Pick from full patient list"
    };
    int how = runVisualRoleMenu("Unlock Patient", "Unlock patient account", "How do you want to find the patient?", howLabels, 2, false);
    if (how == 0) return;
    int patientId = 0;
    if (how == 1) {
        if (!getVisualIntInput("Unlock Patient", "Enter Patient ID to unlock", patientId, true)) return;
    }
    else {
        Patient* allP = patients.getAll();
        int ids[100];
        char rows[100][220];
        const char* rptr[100];
        int n = patients.size();
        if (n > 100) n = 100;
        for (int i = 0; i < n; i++) {
            ids[i] = allP[i].getId();
            DynBuf ln;
            dynBufInit(&ln);
            dynBufAppendStr(&ln, "ID ");
            dynBufAppendInt(&ln, allP[i].getId());
            dynBufAppendStr(&ln, " | ");
            dynBufAppendStr(&ln, allP[i].getName());
            stringCopy(rows[i], dynBufCStr(&ln));
            dynBufFree(&ln);
            rptr[i] = rows[i];
        }
        if (n == 0) { showVisualMessage("Unlock Patient", "No patients loaded."); return; }
        int pick = runVisualScrollPicker("Unlock Patient", "Select patient to unlock", rptr, n);
        if (pick == 0) return;
        patientId = ids[pick - 1];
    }
    if (patients.findById(patientId, getPatId) == nullptr) {
        showVisualMessage("Unlock Patient", "Patient not found.");
        return;
    }
    if (FileHandler::unlockAccount("Patient", patientId)) {
        showVisualMessage("Unlock Patient", "Patient account unlocked successfully.");
    }
    else {
        FileHandler::resetFailedLogin("Patient", patientId);
        showVisualMessage("Unlock Patient", "Patient account reset successfully.");
    }
}

void unlockDoctorAccount(Storage<Doctor>& doctors) {
    const char* howLabels[] = {
        "1. Type Doctor ID",
        "2. Pick specialization, then pick doctor from list"
    };
    int how = runVisualRoleMenu("Unlock Doctor", "Unlock doctor account", "How do you want to find the doctor?", howLabels, 2, false);
    if (how == 0) return;
    int doctorId = 0;
    if (how == 1) {
        if (!getVisualIntInput("Unlock Doctor", "Enter Doctor ID to unlock", doctorId, true)) return;
    }
    else {
        Doctor* allD = doctors.getAll();
        char uniqueSpecs[100][100];
        int specCount = 0;
        for (int i = 0; i < doctors.size(); i++) {
            const char* spec = allD[i].Getspecialization();
            bool exists = false;
            for (int j = 0; j < specCount; j++) {
                if (stringCompare(uniqueSpecs[j], spec)) { exists = true; break; }
            }
            if (!exists && specCount < 100) {
                int k = 0;
                while (spec[k] != '\0' && k < 99) { uniqueSpecs[specCount][k] = spec[k]; k++; }
                uniqueSpecs[specCount][k] = '\0';
                specCount++;
            }
        }
        if (specCount == 0) { showVisualMessage("Unlock Doctor", "No doctors in system."); return; }
        const char* specPtrs[100];
        for (int i = 0; i < specCount; i++) specPtrs[i] = uniqueSpecs[i];
        int specPick = runVisualScrollPicker("Unlock Doctor", "Choose specialization to filter doctors", specPtrs, specCount);
        if (specPick == 0) return;
        char specialization[100];
        int s = 0;
        while (uniqueSpecs[specPick - 1][s] != '\0' && s < 99) {
            specialization[s] = uniqueSpecs[specPick - 1][s];
            s++;
        }
        specialization[s] = '\0';
        int matchIds[100];
        char docRows[100][220];
        const char* dptrs[100];
        int matchCount = 0;
        for (int i = 0; i < doctors.size(); i++) {
            if (!stringCompare(allD[i].Getspecialization(), specialization)) continue;
            matchIds[matchCount] = allD[i].getId();
            DynBuf ln;
            dynBufInit(&ln);
            dynBufAppendStr(&ln, "ID ");
            dynBufAppendInt(&ln, allD[i].getId());
            dynBufAppendStr(&ln, " | ");
            dynBufAppendStr(&ln, allD[i].getName());
            dynBufAppendStr(&ln, " | PKR ");
            dynBufAppendFloat2(&ln, allD[i].Getfee());
            stringCopy(docRows[matchCount], dynBufCStr(&ln));
            dynBufFree(&ln);
            dptrs[matchCount] = docRows[matchCount];
            matchCount++;
        }
        if (matchCount == 0) { showVisualMessage("Unlock Doctor", "No doctors in that specialization."); return; }
        int docPick = runVisualScrollPicker("Unlock Doctor", "Choose doctor to unlock", dptrs, matchCount);
        if (docPick == 0) return;
        doctorId = matchIds[docPick - 1];
    }
    if (doctors.findById(doctorId, getDocId) == nullptr) {
        showVisualMessage("Unlock Doctor", "Doctor not found.");
        return;
    }
    if (FileHandler::unlockAccount("Doctor", doctorId)) {
        showVisualMessage("Unlock Doctor", "Doctor account unlocked successfully.");
    }
    else {
        FileHandler::resetFailedLogin("Doctor", doctorId);
        showVisualMessage("Unlock Doctor", "Doctor account reset successfully.");
    }
}

void removeDoctor(Storage<Doctor>& doctors, Storage<Appointment>& appointments) {
    Doctor* allDoctors = doctors.getAll();
    DynBuf content;
    dynBufInit(&content);
    dynBufAppendStr(&content, "Doctor ID | Name | Specialization | Fee\n");
    for (int i = 0; i < doctors.size(); i++) {
        dynBufAppendInt(&content, allDoctors[i].getId());
        dynBufAppendStr(&content, " | ");
        dynBufAppendStr(&content, allDoctors[i].getName());
        dynBufAppendStr(&content, " | ");
        dynBufAppendStr(&content, allDoctors[i].Getspecialization());
        dynBufAppendStr(&content, " | ");
        dynBufAppendFloat2(&content, allDoctors[i].Getfee());
        dynBufAppendStr(&content, "\n");
    }
    showVisualTextWindow("Remove Doctor", "Current Doctors", dynBufCStr(&content));
    dynBufFree(&content);
    int doctorId; if (!getVisualIntInput("Remove Doctor", "Enter Doctor ID to remove", doctorId, true)) return;
    if (doctors.findById(doctorId, getDocId) == nullptr) {
        showVisualMessage("Remove Doctor", "Doctor not found.");
        return;
    }
    Appointment* allAppointments = appointments.getAll();
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getdocid() == doctorId && stringCompare(allAppointments[i].getstatus(), "pending")) {
            showVisualMessage("Remove Doctor", "Cannot remove doctor with pending appointments. Cancel or reassign them first."); return;
        }
    }
    FileHandler::deleteDoctor(doctorId);
    doctors.removeById(doctorId, getDocId);
    showVisualMessage("Remove Doctor", "Doctor removed.");
}

void viewAllPatients(Storage<Patient>& patients, Storage<Bill>& bills) {
    Patient* allPatients = patients.getAll();
    DynBuf content;
    dynBufInit(&content);
    dynBufAppendStr(&content, "ID | Name | Age | Gender | Contact | Balance | Unpaid Bills\n");
    for (int i = 0; i < patients.size(); i++) {
        int unpaidCount = 0;
        Bill* allBills = bills.getAll();
        for (int j = 0; j < bills.size(); j++) {
            if (allBills[j].getpatid() == allPatients[i].getId() && stringCompare(allBills[j].getstatus(), "unpaid"))
                unpaidCount++;
        }
        dynBufAppendInt(&content, allPatients[i].getId());
        dynBufAppendStr(&content, " | ");
        dynBufAppendStr(&content, allPatients[i].getName());
        dynBufAppendStr(&content, " | ");
        dynBufAppendInt(&content, allPatients[i].getAge());
        dynBufAppendStr(&content, " | ");
        char g[2]; g[0] = allPatients[i].getGender(); g[1] = '\0';
        dynBufAppendStr(&content, g);
        dynBufAppendStr(&content, " | ");
        dynBufAppendStr(&content, allPatients[i].getContact());
        dynBufAppendStr(&content, " | PKR ");
        dynBufAppendFloat2(&content, allPatients[i].getBalance());
        dynBufAppendStr(&content, " | Unpaid Bills: ");
        dynBufAppendInt(&content, unpaidCount);
        dynBufAppendStr(&content, "\n");
    }
    showVisualTextWindow("All Patients", "Patients", dynBufCStr(&content));
    dynBufFree(&content);
}

void viewAllDoctors(Storage<Doctor>& doctors) {
    Doctor* allDoctors = doctors.getAll();
    DynBuf content;
    dynBufInit(&content);
    dynBufAppendStr(&content, "ID | Name | Specialization | Contact | Fee\n");
    for (int i = 0; i < doctors.size(); i++) {
        dynBufAppendInt(&content, allDoctors[i].getId());
        dynBufAppendStr(&content, " | ");
        dynBufAppendStr(&content, allDoctors[i].getName());
        dynBufAppendStr(&content, " | ");
        dynBufAppendStr(&content, allDoctors[i].Getspecialization());
        dynBufAppendStr(&content, " | ");
        dynBufAppendStr(&content, allDoctors[i].Getcontact());
        dynBufAppendStr(&content, " | PKR ");
        dynBufAppendFloat2(&content, allDoctors[i].Getfee());
        dynBufAppendStr(&content, "\n");
    }
    showVisualTextWindow("All Doctors", "Doctors", dynBufCStr(&content));
    dynBufFree(&content);
}

void viewAllAppointments(Storage<Appointment>& appointments, Storage<Patient>& patients, Storage<Doctor>& doctors) {
    Appointment* allAppointments = appointments.getAll();
    DynBuf content;
    dynBufInit(&content);
    dynBufAppendStr(&content, "ID | Patient Name | Doctor Name | Date | Time Slot | Status\n");
    for (int i = 0; i < appointments.size(); i++) {
        char patientName[100] = "Unknown", doctorName[100] = "Unknown";
        Patient* allPatients = patients.getAll();
        for (int j = 0; j < patients.size(); j++) {
            if (allPatients[j].getId() == allAppointments[i].getpatid()) {
                int k = 0;
                while (allPatients[j].getName()[k] != '\0') { patientName[k] = allPatients[j].getName()[k]; k++; }
                patientName[k] = '\0'; break;
            }
        }
        Doctor* allDoctors = doctors.getAll();
        for (int j = 0; j < doctors.size(); j++) {
            if (allDoctors[j].getId() == allAppointments[i].getdocid()) {
                int k = 0;
                while (allDoctors[j].getName()[k] != '\0') { doctorName[k] = allDoctors[j].getName()[k]; k++; }
                doctorName[k] = '\0'; break;
            }
        }
        dynBufAppendInt(&content, allAppointments[i].getappid());
        dynBufAppendStr(&content, " | ");
        dynBufAppendStr(&content, patientName);
        dynBufAppendStr(&content, " | ");
        dynBufAppendStr(&content, doctorName);
        dynBufAppendStr(&content, " | ");
        dynBufAppendStr(&content, allAppointments[i].getdate());
        dynBufAppendStr(&content, " | ");
        dynBufAppendStr(&content, allAppointments[i].gettime());
        dynBufAppendStr(&content, " | ");
        dynBufAppendStr(&content, allAppointments[i].getstatus());
        dynBufAppendStr(&content, "\n");
    }
    showVisualTextWindow("All Appointments", "Appointments", dynBufCStr(&content));
    dynBufFree(&content);
}

void viewUnpaidBills(Storage<Bill>& bills, Storage<Patient>& patients) {
    Bill* allBills = bills.getAll();
    DynBuf content;
    dynBufInit(&content);
    dynBufAppendStr(&content, "Bill ID | Patient Name | Amount (PKR) | Date\n");
    for (int i = 0; i < bills.size(); i++) {
        if (stringCompare(allBills[i].getstatus(), "unpaid")) {
            char patientName[100] = "Unknown";
            Patient* allPatients = patients.getAll();
            for (int j = 0; j < patients.size(); j++) {
                if (allPatients[j].getId() == allBills[i].getpatid()) {
                    int k = 0;
                    while (allPatients[j].getName()[k] != '\0') { patientName[k] = allPatients[j].getName()[k]; k++; }
                    patientName[k] = '\0'; break;
                }
            }
            dynBufAppendInt(&content, allBills[i].getbillid());
            dynBufAppendStr(&content, " | ");
            dynBufAppendStr(&content, patientName);
            dynBufAppendStr(&content, " | PKR ");
            dynBufAppendFloat2(&content, allBills[i].getamount());
            dynBufAppendStr(&content, " | ");
            dynBufAppendStr(&content, allBills[i].getdate());
            dynBufAppendStr(&content, "\n");
        }
    }
    showVisualTextWindow("Unpaid Bills", "Unpaid Bills", dynBufCStr(&content));
    dynBufFree(&content);
}

void dischargePatientMenu(Storage<Patient>& patients, Storage<Appointment>& appointments, Storage<Bill>& bills, Storage<Prescription>& prescriptions) {
    int patientId; if (!getVisualIntInput("Discharge Patient", "Enter Patient ID", patientId, true)) return;
    if (patients.findById(patientId, getPatId) == nullptr) {
        showVisualMessage("Discharge Patient", "Patient not found.");
        return;
    }
    Bill* allBills = bills.getAll();
    for (int i = 0; i < bills.size(); i++) {
        if (allBills[i].getpatid() == patientId && stringCompare(allBills[i].getstatus(), "unpaid")) {
            showVisualMessage("Discharge Patient", "Cannot discharge patient with unpaid bills."); return;
        }
    }
    Appointment* allAppointments = appointments.getAll();
    for (int i = 0; i < appointments.size(); i++) {
        if (allAppointments[i].getpatid() == patientId && stringCompare(allAppointments[i].getstatus(), "pending")) {
            showVisualMessage("Discharge Patient", "Cannot discharge patient with pending appointments."); return;
        }
    }
    FileHandler::dischargePatient(patientId, patients, appointments, bills, prescriptions);
    FileHandler::loadPatients(patients);
    FileHandler::loadAppointments(appointments);
    FileHandler::loadBills(bills);
    FileHandler::loadPrescriptions(prescriptions);
    showVisualMessage("Discharge Patient", "Patient discharged and archived successfully.");
}

void viewSecurityLog() {
    ifstream securityFile("security_log.txt");
    if (!securityFile.is_open()) { showVisualMessage("Security Log", "No security events logged."); return; }
    char line[500];
    bool empty = true;
    DynBuf content;
    dynBufInit(&content);
    dynBufAppendStr(&content, "Timestamp | Role | Entered ID | Result\n");
    while (securityFile.getline(line, 500)) {
        if (line[0] == '\0') continue;
        if (stringCompare(line, "timestamp,role,entered_id,result")) continue;
        dynBufAppendStr(&content, line);
        dynBufAppendStr(&content, "\n");
        empty = false;
    }
    if (empty) { showVisualMessage("Security Log", "No security events logged."); dynBufFree(&content); }
    else { showVisualTextWindow("Security Log", "Security Events", dynBufCStr(&content)); dynBufFree(&content); }
    securityFile.close();
}

void generateDailyReport(Storage<Appointment>& appointments, Storage<Bill>& bills, Storage<Patient>& patients, Storage<Doctor>& doctors) {
    char today[11]; getCurrentDate(today);
    int total = 0, pending = 0, completed = 0, noshow = 0, cancelled = 0;
    float revenue = 0;
    Appointment* allAppointments = appointments.getAll();
    for (int i = 0; i < appointments.size(); i++) {
        if (stringCompare(allAppointments[i].getdate(), today)) {
            total++;
            if (stringCompare(allAppointments[i].getstatus(), "pending")) pending++;
            else if (stringCompare(allAppointments[i].getstatus(), "completed")) completed++;
            else if (stringCompare(allAppointments[i].getstatus(), "noshow")) noshow++;
            else if (stringCompare(allAppointments[i].getstatus(), "cancelled")) cancelled++;
        }
    }
    Bill* allBills = bills.getAll();
    for (int i = 0; i < bills.size(); i++) {
        if (stringCompare(allBills[i].getdate(), today) && stringCompare(allBills[i].getstatus(), "paid"))
            revenue += allBills[i].getamount();
    }
    DynBuf content;
    dynBufInit(&content);
    dynBufAppendStr(&content, "Daily Report: ");
    dynBufAppendStr(&content, today);
    dynBufAppendStr(&content, "\n\nTotal appointments today: ");
    dynBufAppendInt(&content, total);
    dynBufAppendStr(&content, " (Pending: ");
    dynBufAppendInt(&content, pending);
    dynBufAppendStr(&content, " Completed: ");
    dynBufAppendInt(&content, completed);
    dynBufAppendStr(&content, " No-show: ");
    dynBufAppendInt(&content, noshow);
    dynBufAppendStr(&content, " Cancelled: ");
    dynBufAppendInt(&content, cancelled);
    dynBufAppendStr(&content, ")\nRevenue collected today (paid bills): PKR ");
    dynBufAppendFloat2(&content, revenue);
    dynBufAppendStr(&content, "\n\nPatients with outstanding unpaid bills:\n");
    Patient* allPatients = patients.getAll();
    for (int i = 0; i < patients.size(); i++) {
        float totalOwed = 0;
        for (int j = 0; j < bills.size(); j++) {
            if (allBills[j].getpatid() == allPatients[i].getId() && stringCompare(allBills[j].getstatus(), "unpaid"))
                totalOwed += allBills[j].getamount();
        }
        if (totalOwed > 0) {
            dynBufAppendStr(&content, allPatients[i].getName());
            dynBufAppendStr(&content, " | Total Owed: PKR ");
            dynBufAppendFloat2(&content, totalOwed);
            dynBufAppendStr(&content, "\n");
        }
    }
    dynBufAppendStr(&content, "\nDoctor-wise summary for today:\n");
    Doctor* allDoctors = doctors.getAll();
    for (int i = 0; i < doctors.size(); i++) {
        int docCompleted = 0, docPending = 0, docNoshow = 0;
        for (int j = 0; j < appointments.size(); j++) {
            if (allAppointments[j].getdocid() == allDoctors[i].getId() && stringCompare(allAppointments[j].getdate(), today)) {
                if (stringCompare(allAppointments[j].getstatus(), "completed")) docCompleted++;
                else if (stringCompare(allAppointments[j].getstatus(), "pending")) docPending++;
                else if (stringCompare(allAppointments[j].getstatus(), "noshow")) docNoshow++;
            }
        }
        dynBufAppendStr(&content, allDoctors[i].getName());
        dynBufAppendStr(&content, " | Completed: ");
        dynBufAppendInt(&content, docCompleted);
        dynBufAppendStr(&content, " | Pending: ");
        dynBufAppendInt(&content, docPending);
        dynBufAppendStr(&content, " | No-show: ");
        dynBufAppendInt(&content, docNoshow);
        dynBufAppendStr(&content, "\n");
    }
    showVisualTextWindow("Daily Report", "Hospital Daily Report", dynBufCStr(&content));
    dynBufFree(&content);
}

void adminMenu(Admin& admin, Storage<Patient>& patients, Storage<Doctor>& doctors, Storage<Appointment>& appointments, Storage<Bill>& bills, Storage<Prescription>& prescriptions) {
    int choice;
    const char* options[] = {
        "1. Add Doctor",
        "2. Remove Doctor",
        "3. View All Patients",
        "4. View All Doctors",
        "5. View All Appointments",
        "6. View Unpaid Bills",
        "7. Discharge Patient",
        "8. View Security Log",
        "9. Generate Daily Report",
        "10. Unlock Patient Account",
        "11. Unlock Doctor Account",
        "12. Logout"
    };
    while (true) {
        choice = runVisualRoleMenu("Admin Menu", "Admin Dashboard", "MediCore hospital administration controls", options, 12);
        if (choice <= 0) continue;
        if (choice == 1) addDoctor(doctors);
        else if (choice == 2) removeDoctor(doctors, appointments);
        else if (choice == 3) viewAllPatients(patients, bills);
        else if (choice == 4) viewAllDoctors(doctors);
        else if (choice == 5) viewAllAppointments(appointments, patients, doctors);
        else if (choice == 6) viewUnpaidBills(bills, patients);
        else if (choice == 7) dischargePatientMenu(patients, appointments, bills, prescriptions);
        else if (choice == 8) viewSecurityLog();
        else if (choice == 9) generateDailyReport(appointments, bills, patients, doctors);
        else if (choice == 10) unlockPatientAccount(patients);
        else if (choice == 11) unlockDoctorAccount(doctors);
        else if (choice == 12) break;
        else showVisualMessage("Admin Menu", "Invalid option.");
    }
}

// ==================== MAIN ====================

int main() {
    Storage<Patient> patients;
    Storage<Doctor> doctors;
    Storage<Appointment> appointments;
    Storage<Bill> bills;
    Storage<Prescription> prescriptions;
    Admin admin;

    FileHandler::initializeFiles();
    try {
        FileHandler::loadPatients(patients);
        FileHandler::loadDoctors(doctors);
        FileHandler::loadAdmin(admin);
        FileHandler::loadAppointments(appointments);
        FileHandler::loadBills(bills);
        FileHandler::loadPrescriptions(prescriptions);
    }
    catch (HospitalException& e) {
        showVisualMessage("Startup Error", e.what());
    }

    int choice = 0;
    while (true) {
        choice = getVisualMainMenuChoice();
        if (choice == 0) continue;

        if (choice == 1) {
            int attempts = 0;
            while (attempts < 3) {
                int id = 0; char pass[50];
                if (!getVisualCredentials(id, pass, 50)) break;
                if (FileHandler::isAccountLocked("Patient", id)) {
                    FileHandler::logSecurity("Patient", id, "LOCKED");
                    break;
                }
                Patient* p = patients.findById(id, getPatId);
                if (p && stringCompare(pass, p->getPassword())) {
                    FileHandler::resetFailedLogin("Patient", id);
                    FileHandler::logSecurity("Patient", id, "LoginSuccessful");
                    patientMenu(p, doctors, appointments, bills, prescriptions);
                    break;
                }
                attempts++;
                FileHandler::recordFailedLogin("Patient", id);
                FileHandler::logSecurity("Patient", id, "FAILED");
                DynBuf msg;
                dynBufInit(&msg);
                dynBufAppendStr(&msg, "Invalid credentials.\nAttempts left: ");
                dynBufAppendInt(&msg, 3 - attempts);
                if (attempts == 3) dynBufAppendStr(&msg, "\nAccount locked. Contact admin.");
                showVisualTextWindow("Patient Login", "Login Failed", dynBufCStr(&msg));
                dynBufFree(&msg);
            }
        }
        else if (choice == 2) {
            int attempts = 0;
            while (attempts < 3) {
                int id = 0; char pass[50];
                if (!getVisualCredentials(id, pass, 50)) break;
                if (FileHandler::isAccountLocked("Doctor", id)) {
                    FileHandler::logSecurity("Doctor", id, "LOCKED");
                    break;
                }
                Doctor* d = doctors.findById(id, getDocId);
                if (d && stringCompare(pass, d->getPassword())) {
                    FileHandler::resetFailedLogin("Doctor", id);
                    FileHandler::logSecurity("Doctor", id, "LoginSuccessful");
                    doctorMenu(d, patients, appointments, bills, prescriptions);
                    break;
                }
                attempts++;
                FileHandler::recordFailedLogin("Doctor", id);
                FileHandler::logSecurity("Doctor", id, "FAILED");
                DynBuf msg;
                dynBufInit(&msg);
                dynBufAppendStr(&msg, "Invalid credentials.\nAttempts left: ");
                dynBufAppendInt(&msg, 3 - attempts);
                if (attempts == 3) dynBufAppendStr(&msg, "\nAccount locked. Contact admin.");
                showVisualTextWindow("Doctor Login", "Login Failed", dynBufCStr(&msg));
                dynBufFree(&msg);
            }
        }
        else if (choice == 3) {
            int attempts = 0;
            while (attempts < 3) {
                int id = 0; char pass[50];
                if (!getVisualCredentials(id, pass, 50)) break;
                if (id == admin.getId() && stringCompare(pass, admin.getPassword())) {
                    FileHandler::logSecurity("Admin", id, "LoginSuccessful");
                    adminMenu(admin, patients, doctors, appointments, bills, prescriptions);
                    break;
                }
                attempts++;
                FileHandler::logSecurity("Admin", id, "FAILED");
                DynBuf msg;
                dynBufInit(&msg);
                dynBufAppendStr(&msg, "Invalid credentials.\nAttempts left: ");
                dynBufAppendInt(&msg, 3 - attempts);
                if (attempts == 3) dynBufAppendStr(&msg, "\nAccount locked. Contact admin.");
                showVisualTextWindow("Admin Login", "Login Failed", dynBufCStr(&msg));
                dynBufFree(&msg);
            }
        }
        else if (choice == 4) registerPatientVisual(patients);
        else if (choice == 5) break;
        else showVisualMessage("Main Menu", "Invalid choice.");
    }
    showVisualMessage("MediCore", "Thank you for using MediCore. Goodbye!");
    return 0;
}