#pragma once

class Rectangle {

public:
    int x1;
    int y1;
    int x2;
    int y2;

    Rectangle(int x, int y, int width, int height);

    bool contains(int x, int y) const;
};