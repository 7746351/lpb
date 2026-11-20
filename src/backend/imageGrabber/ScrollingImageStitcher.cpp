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

#include "ScrollingImageStitcher.h"

#include <QPainter>

namespace {
constexpr int ComparisonWidth = 240;
constexpr double MaximumOverlapError = 0.08;
constexpr double IdenticalFrameError = 0.01;
constexpr int IdenticalFrameTolerance = 3;
constexpr int MinimumOverlapHeight = 16;
}

ScrollingImageStitcher::ScrollingImageStitcher(int maxHeight) :
	mHeight(0),
	mMaxHeight(maxHeight)
{
}

void ScrollingImageStitcher::reset()
{
	mCanvas = QImage();
	mHeight = 0;
}

ScrollingImageStitcher::Status ScrollingImageStitcher::addFrame(const QImage &frame)
{
	if (frame.isNull()) {
		return Status::Failed;
	}

	const auto normalizedFrame = frame.convertToFormat(QImage::Format_RGB32);

	if (mCanvas.isNull()) {
		mCanvas = QImage(normalizedFrame.width(), mMaxHeight, QImage::Format_RGB32);
		mHeight = 0;

		const auto initialHeight = qMin(normalizedFrame.height(), mMaxHeight);
		QPainter painter(&mCanvas);
		painter.drawImage(0, 0, normalizedFrame, 0, 0, normalizedFrame.width(), initialHeight);
		painter.end();

		mHeight = initialHeight;
		return Status::FirstFrame;
	}

	if (mCanvas.width() != normalizedFrame.width()) {
		return Status::Failed;
	}

	if (mHeight >= mMaxHeight) {
		return Status::Duplicate;
	}

	const auto minimumOverlap = qMax(MinimumOverlapHeight, normalizedFrame.height() / 8);
	const auto maximumOverlap = qMin(normalizedFrame.height(), mHeight);

	if (maximumOverlap <= minimumOverlap) {
		return Status::Failed;
	}

	const auto tailHeight = qMin(normalizedFrame.height(), mHeight);
	const auto comparisonImage = prepareForComparison(mCanvas.copy(0, mHeight - tailHeight, mCanvas.width(), tailHeight));
	const auto comparisonFrame = prepareForComparison(normalizedFrame);
	const auto comparisonOverlapMin = qMax(1, qRound(minimumOverlap * comparisonFrame.height() / static_cast<double>(normalizedFrame.height())));
	const auto comparisonOverlapMax = qMin(comparisonFrame.height(), qRound(maximumOverlap * comparisonFrame.height() / static_cast<double>(normalizedFrame.height())));

	auto bestOverlap = -1;
	auto bestError = 1.0;

	for (auto overlap = comparisonOverlapMax; overlap >= comparisonOverlapMin; --overlap) {
		const auto error = calculateError(comparisonImage, comparisonFrame, overlap);
		if (error < bestError) {
			bestError = error;
			bestOverlap = overlap;
			if (error <= IdenticalFrameError) {
				break;
			}
		}
	}

	if (bestOverlap < 0 || bestError > MaximumOverlapError) {
		return Status::Failed;
	}

	const auto scaledOverlap = qRound(bestOverlap * normalizedFrame.height() / static_cast<double>(comparisonFrame.height()));

	if (scaledOverlap >= normalizedFrame.height() - IdenticalFrameTolerance && bestError <= IdenticalFrameError) {
		return Status::Duplicate;
	}

	const auto appendHeight = qMin(normalizedFrame.height() - scaledOverlap, mMaxHeight - mHeight);
	if (appendHeight <= 0) {
		return Status::Duplicate;
	}

	QPainter painter(&mCanvas);
	painter.drawImage(0, mHeight, normalizedFrame, 0, scaledOverlap, normalizedFrame.width(), appendHeight);
	painter.end();

	mHeight += appendHeight;

	return Status::Appended;
}

QImage ScrollingImageStitcher::image() const
{
	if (mCanvas.isNull() || mHeight <= 0) {
		return QImage();
	}

	return mCanvas.copy(0, 0, mCanvas.width(), mHeight);
}

int ScrollingImageStitcher::height() const
{
	return mHeight;
}

bool ScrollingImageStitcher::isEmpty() const
{
	return mCanvas.isNull() || mHeight <= 0;
}

QImage ScrollingImageStitcher::prepareForComparison(const QImage &image)
{
	if (image.width() <= ComparisonWidth) {
		return image;
	}

	return image.scaledToWidth(ComparisonWidth, Qt::SmoothTransformation);
}

double ScrollingImageStitcher::calculateError(const QImage &image, const QImage &frame, int overlap)
{
	if (overlap <= 0 || overlap > image.height() || overlap > frame.height()) {
		return 1.0;
	}

	const auto width = image.width();
	const auto rowStep = qMax(1, overlap / 64);
	double totalError = 0;
	int comparedRows = 0;

	for (auto row = 0; row < overlap; row += rowStep) {
		const auto imageRow = image.height() - overlap + row;
		const auto imageLine = reinterpret_cast<const QRgb *>(image.constScanLine(imageRow));
		const auto frameLine = reinterpret_cast<const QRgb *>(frame.constScanLine(row));

		for (auto column = 0; column < width; ++column) {
			const auto imagePixel = imageLine[column];
			const auto framePixel = frameLine[column];
			totalError += qAbs(qRed(imagePixel) - qRed(framePixel));
			totalError += qAbs(qGreen(imagePixel) - qGreen(framePixel));
			totalError += qAbs(qBlue(imagePixel) - qBlue(framePixel));
		}

		++comparedRows;
	}

	if (comparedRows == 0) {
		return 1.0;
	}

	return totalError / (comparedRows * width * 3.0 * 255.0);
}
