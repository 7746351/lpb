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

#ifndef KSNIP_SCROLLINGIMAGESTITCHER_H
#define KSNIP_SCROLLINGIMAGESTITCHER_H

#include <QImage>

class ScrollingImageStitcher
{
public:
	enum class Status
	{
		FirstFrame,
		Appended,
		Duplicate,
		Failed
	};

	explicit ScrollingImageStitcher(int maxHeight = 10000);
	~ScrollingImageStitcher() = default;

	void reset();
	Status addFrame(const QImage &frame);

	QImage image() const;
	int height() const;
	bool isEmpty() const;

private:
	QImage mCanvas;
	int mHeight;
	int mMaxHeight;

	static QImage prepareForComparison(const QImage &image);
	static double calculateError(const QImage &image, const QImage &frame, int overlap);
};

#endif //KSNIP_SCROLLINGIMAGESTITCHER_H
