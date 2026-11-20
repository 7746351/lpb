/*
 * Copyright (C) 2021 Damir Porobic <damir.porobic@gmx.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KSNIP_SCROLLINGIMAGESTITCHERTESTS_H
#define KSNIP_SCROLLINGIMAGESTITCHERTESTS_H

#include <QtTest>

class ScrollingImageStitcherTests : public QObject
{
	Q_OBJECT
private slots:
	void AddFrame_Should_AppendNewContent_When_FrameOverlapsExistingImage();
	void AddFrame_Should_ReturnDuplicate_When_FrameAlreadyContained();
	void AddFrame_Should_ReturnFailed_When_FrameWidthDiffers();
};

#endif //KSNIP_SCROLLINGIMAGESTITCHERTESTS_H
