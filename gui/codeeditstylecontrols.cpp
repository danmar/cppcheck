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

#include "codeeditstylecontrols.h"
#include <QColorDialog>

SelectColorButton::SelectColorButton(QWidget* parent) :
    QPushButton(parent),
    mColor(QColor(255, 255, 255))
{
    updateColor();
    connect(this, SIGNAL(clicked()), this, SLOT(changeColor()));
}

void SelectColorButton::updateColor()
{
    QString btnColorStyle = QString(
                                "background-color:rgb(%1,%2,%3);"
                                "border-style:outset;"
                                "border-width: 1px;")
                            .arg(mColor.red())
                            .arg(mColor.green())
                            .arg(mColor.blue());
    setObjectName("SelectColorButton");
    setStyleSheet(btnColorStyle);
}

void SelectColorButton::changeColor()
{
    QColorDialog pDlg(mColor);
    pDlg.setModal(true);
    int nResult = pDlg.exec();
    if (nResult == QDialog::Accepted) {
        setColor(pDlg.selectedColor());
        emit colorChanged(mColor);
    }
}

void SelectColorButton::setColor(const QColor& color)
{
    mColor = color;
    updateColor();
}

const QColor& SelectColorButton::getColor()
{
    return mColor;
}

SelectFontWeightCombo::SelectFontWeightCombo(QWidget* parent) :
    QComboBox(parent),
    mWeight(QFont::Normal)
{
    addItem(QObject::tr("Thin"),
            QVariant(static_cast<int>(QFont::Thin)));
    addItem(QObject::tr("ExtraLight"),
            QVariant(static_cast<int>(QFont::ExtraLight)));
    addItem(QObject::tr("Light"),
            QVariant(static_cast<int>(QFont::Light)));
    addItem(QObject::tr("Normal"),
            QVariant(static_cast<int>(QFont::Normal)));
    addItem(QObject::tr("Medium"),
            QVariant(static_cast<int>(QFont::Medium)));
    addItem(QObject::tr("DemiBold"),
            QVariant(static_cast<int>(QFont::DemiBold)));
    addItem(QObject::tr("Bold"),
            QVariant(static_cast<int>(QFont::Bold)));
    addItem(QObject::tr("ExtraBold"),
            QVariant(static_cast<int>(QFont::ExtraBold)));
    addItem(QObject::tr("Black"),
            QVariant(static_cast<int>(QFont::Black)));
    updateWeight();
    connect(this, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeWeight(int)));
}

void SelectFontWeightCombo::updateWeight()
{
    int nResult = findData(QVariant(static_cast<int>(mWeight)));

    if (nResult != -1) {
        setCurrentIndex(nResult);
    } else {
        setCurrentIndex(findData(static_cast<int>(QFont::Normal)));
    }
}

void SelectFontWeightCombo::changeWeight(int index)
{
    if (index != -1) {
        setWeight(static_cast<QFont::Weight>(itemData(index).toInt()));
        emit weightChanged(mWeight);
    }
}

void SelectFontWeightCombo::setWeight(const QFont::Weight& weight)
{
    mWeight = weight;
    updateWeight();
}

const QFont::Weight& SelectFontWeightCombo::getWeight()
{
    return mWeight;
}
