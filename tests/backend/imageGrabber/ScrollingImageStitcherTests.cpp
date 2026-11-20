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

#include "ScrollingImageStitcherTests.h"

#include "src/backend/imageGrabber/ScrollingImageStitcher.h"

#include "tests/utils/TestRunner.h"

namespace {
QImage createPage(int width, int height)
{
	QImage page(width, height, QImage::Format_RGB32);

	for (auto y = 0; y < height; ++y) {
		for (auto x = 0; x < width; ++x) {
			page.setPixelColor(x, y, QColor((y * 7 + x) % 256, (y * 13 + x * 3) % 256, (y * 29 + x * 5) % 256));
		}
	}

	return page;
}

QImage cropFrame(const QImage &page, int top, int height)
{
	return page.copy(0, top, page.width(), height);
}
}

void ScrollingImageStitcherTests::AddFrame_Should_AppendNewContent_When_FrameOverlapsExistingImage()
{
	// arrange
	const auto page = createPage(64, 200);
	const auto firstFrame = cropFrame(page, 0, 100);
	const auto secondFrame = cropFrame(page, 60, 100);
	const auto thirdFrame = cropFrame(page, 120, 80);

	ScrollingImageStitcher stitcher;

	// act
	QCOMPARE(stitcher.addFrame(firstFrame), ScrollingImageStitcher::Status::FirstFrame);
	QCOMPARE(stitcher.addFrame(secondFrame), ScrollingImageStitcher::Status::Appended);
	QCOMPARE(stitcher.addFrame(thirdFrame), ScrollingImageStitcher::Status::Appended);

	// assert
	QCOMPARE(stitcher.height(), page.height());
	QCOMPARE(stitcher.image(), page);
}

void ScrollingImageStitcherTests::AddFrame_Should_ReturnDuplicate_When_FrameAlreadyContained()
{
	// arrange
	const auto page = createPage(64, 120);
	const auto frame = cropFrame(page, 0, 120);

	ScrollingImageStitcher stitcher;
	QCOMPARE(stitcher.addFrame(frame), ScrollingImageStitcher::Status::FirstFrame);
	const auto initialHeight = stitcher.height();

	// act
	const auto status = stitcher.addFrame(frame);

	// assert
	QCOMPARE(status, ScrollingImageStitcher::Status::Duplicate);
	QCOMPARE(stitcher.height(), initialHeight);
}

void ScrollingImageStitcherTests::AddFrame_Should_ReturnFailed_When_FrameWidthDiffers()
{
	// arrange
	const auto page = createPage(64, 120);
	const auto narrowerFrame = createPage(48, 60);

	ScrollingImageStitcher stitcher;
	QCOMPARE(stitcher.addFrame(cropFrame(page, 0, 120)), ScrollingImageStitcher::Status::FirstFrame);

	// act
	const auto status = stitcher.addFrame(narrowerFrame);

	// assert
	QCOMPARE(status, ScrollingImageStitcher::Status::Failed);
}

TEST_MAIN(ScrollingImageStitcherTests)
