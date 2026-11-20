/*
 * Copyright (C) 2018 Damir Porobic <https://github.com/damirporobic>
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

#include "WinImageGrabber.h"

#include <QDebug>

namespace {
constexpr int ScrollingTimerIntervalMs = 300;
constexpr int ScrollNotchesPerStep = 3;
constexpr int MaximumScrollingFrames = 60;
constexpr int MaximumScrollingHeight = 10000;
constexpr int MaximumDuplicateFrames = 3;
}

WinImageGrabber::WinImageGrabber(const QSharedPointer<IConfig> &config) :
	AbstractRectAreaImageGrabber(new WinSnippingArea(config), config),
    mWinWrapper(new WinWrapper),
	mScrollingTimer(new QTimer(this)),
	mScrollingStitcher(MaximumScrollingHeight),
	mScrollingFrameCount(0),
	mScrollingDuplicateCount(0),
	mScrollingActive(false)
{
	addSupportedCaptureMode(CaptureModes::RectArea);
	addSupportedCaptureMode(CaptureModes::LastRectArea);
	addSupportedCaptureMode(CaptureModes::FullScreen);
	addSupportedCaptureMode(CaptureModes::ActiveWindow);
	addSupportedCaptureMode(CaptureModes::CurrentScreen);
	addSupportedCaptureMode(CaptureModes::Scrolling);

	mScrollingTimer->setInterval(ScrollingTimerIntervalMs);
	connect(mScrollingTimer, &QTimer::timeout, this, &WinImageGrabber::scrollingTimeout);
}

WinImageGrabber::~WinImageGrabber()
{
	restoreCursor();
	delete mWinWrapper;
}

QRect WinImageGrabber::activeWindowRect() const
{
    auto rect = mWinWrapper->getActiveWindowRect();
    return mHdpiScaler.unscale(rect);
}

QRect WinImageGrabber::fullScreenRect() const
{
    auto rect = mWinWrapper->getFullScreenRect();
    return mHdpiScaler.unscale(rect);
}

CursorDto WinImageGrabber::getCursorWithPosition() const
{
    return mWinWrapper->getCursorWithPosition();
}

void WinImageGrabber::grab()
{
	if (captureMode() == CaptureModes::Scrolling) {
		startScrollingCapture();
		return;
	}

	AbstractRectAreaImageGrabber::grab();
}

void WinImageGrabber::startScrollingCapture()
{
	setCaptureRectFromCorrectSource();

	if (mCaptureRect.isEmpty()) {
		emit canceled();
		return;
	}

	const auto firstFrame = getScreenshotFromRect(mCaptureRect).toImage();
	if (firstFrame.isNull()) {
		emit canceled();
		return;
	}

	mScrollingStitcher.reset();
	mScrollingStitcher.addFrame(firstFrame);
	mScrollingFrameCount = 1;
	mScrollingDuplicateCount = 0;
	mScrollingActive = true;

	mScrollingScreenRect = mHdpiScaler.scale(mCaptureRect);
	mOriginalCursorPosition = mWinWrapper->cursorPosition();
	mWinWrapper->setCursorPosition(mScrollingScreenRect.center());

	mWinWrapper->scrollDown(ScrollNotchesPerStep);
	mScrollingTimer->start();
}

void WinImageGrabber::scrollingTimeout()
{
	if (!mScrollingActive) {
		return;
	}

	if (mScrollingFrameCount > 1 && mWinWrapper->isEscapePressed()) {
		cancelScrollingCapture();
		return;
	}

	const auto frame = getScreenshotFromRect(mCaptureRect).toImage();
	const auto status = mScrollingStitcher.addFrame(frame);
	++mScrollingFrameCount;

	if (status == ScrollingImageStitcher::Status::Failed) {
		qWarning("Scrolling capture aborted, unable to stitch frame.");
		cancelScrollingCapture();
		return;
	}

	if (status == ScrollingImageStitcher::Status::Duplicate) {
		++mScrollingDuplicateCount;
	} else {
		mScrollingDuplicateCount = 0;
	}

	if (mScrollingDuplicateCount >= MaximumDuplicateFrames
		|| mScrollingFrameCount >= MaximumScrollingFrames
		|| mScrollingStitcher.height() >= MaximumScrollingHeight) {
		finishScrollingCapture();
		return;
	}

	if (status != ScrollingImageStitcher::Status::Duplicate) {
		mWinWrapper->scrollDown(ScrollNotchesPerStep);
	}
}

void WinImageGrabber::cancelScrollingCapture()
{
	mScrollingTimer->stop();
	restoreCursor();
	mScrollingActive = false;
	emit canceled();
}

void WinImageGrabber::finishScrollingCapture()
{
	mScrollingTimer->stop();
	restoreCursor();
	mScrollingActive = false;

	const auto image = mScrollingStitcher.image();
	if (image.isNull()) {
		emit canceled();
		return;
	}

	emit finished(CaptureDto(QPixmap::fromImage(image)));
}

void WinImageGrabber::restoreCursor()
{
	if (mScrollingActive) {
		mWinWrapper->setCursorPosition(mOriginalCursorPosition);
	}
}
