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

#include "RecentImagesPathStore.h"

RecentImagesPathStore::RecentImagesPathStore(const QSharedPointer<IImagePathStorage> &imagePathStorage) :
	mImagePathStorage(imagePathStorage),
	mMaxRecentItems(10),
	mMaxHistoryItems(200)
{
	Q_ASSERT(mImagePathStorage != nullptr);

	loadImagesPath();
}

void RecentImagesPathStore::loadImagesPath()
{
	const auto storedImageCount = mImagePathStorage->count();
	for (auto i = 0; i < storedImageCount; ++i) {
		const auto path = mImagePathStorage->load(i);
		if (!path.isEmpty() && !mImagesPathCache.contains(path)) {
			mImagesPathCache.append(path);
		}
	}
}

void RecentImagesPathStore::storeImagePath(const QString &imagePath)
{
	if (imagePath.isEmpty()) {
		return;
	}

	if (mImagesPathCache.contains(imagePath)) {
		return;
	}

	while (mImagesPathCache.size() >= mMaxHistoryItems) {
		mImagesPathCache.removeFirst();
	}

	mImagesPathCache.append(imagePath);
	saveImagesPath();
}

void RecentImagesPathStore::removeImagePath(const QString &imagePath)
{
	if (mImagesPathCache.removeAll(imagePath) > 0) {
		saveImagesPath();
	}
}

void RecentImagesPathStore::saveImagesPath()
{
	mImagePathStorage->clear();
	for (auto i = 0; i < mImagesPathCache.size(); ++i) {
		mImagePathStorage->store(mImagesPathCache.at(i), i);
	}
}

QStringList RecentImagesPathStore::getRecentImagesPath() const
{
	QStringList reversedList;
	const auto recentCount = std::min(mMaxRecentItems, static_cast<int>(mImagesPathCache.size()));
	reversedList.reserve(recentCount);

	for (auto i = mImagesPathCache.size() - 1; i >= mImagesPathCache.size() - recentCount; --i) {
		reversedList.append(mImagesPathCache.at(i));
	}

	return reversedList;
}

QStringList RecentImagesPathStore::getHistoryImagesPath() const
{
	QStringList reversedList = mImagesPathCache;
	std::reverse(reversedList.begin(), reversedList.end());

	return reversedList;
}
