/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// widget subclass methodology derived from here:
// https://stackoverflow.com/questions/18257281/qt-color-picker-widget/43871405#43871405

#ifndef CODEEDITORSTYLECONTROLS_H
#define CODEEDITORSTYLECONTROLS_H

#include <QPushButton>
#include <QComboBox>
#include <QColor>
#include <QFont>

class SelectColorButton : public QPushButton {
    Q_OBJECT
public:
    explicit SelectColorButton(QWidget* parent);
    virtual ~SelectColorButton() {}

    void setColor(const QColor& color);
    const QColor& getColor();

signals:
    void colorChanged(const QColor& newColor);

public slots:
    void updateColor();
    void changeColor();

private:
    QColor mColor;
};


class SelectFontWeightCombo : public QComboBox {
    Q_OBJECT
public:
    explicit SelectFontWeightCombo(QWidget* parent);
    virtual ~SelectFontWeightCombo() {}

    void setWeight(const QFont::Weight& weight);
    const QFont::Weight& getWeight();

signals:
    void weightChanged(const QFont::Weight& newWeight);

public slots:
    void updateWeight();
    void changeWeight(int index);

private:
    QFont::Weight mWeight;
};

#endif  //CODEEDITORSTYLECONTROLS_H

