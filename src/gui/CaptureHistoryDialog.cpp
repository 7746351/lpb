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

#include "CaptureHistoryDialog.h"

#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImageReader>
#include <QPixmap>
#include <QVBoxLayout>

#include "src/common/helper/FileDialogFilterHelper.h"

namespace {
constexpr int ThumbnailWidth = 180;
constexpr int ThumbnailHeight = 120;

QIcon createThumbnail(const QString &path)
{
	QImageReader reader(path);
	reader.setAutoTransform(true);

	const auto originalSize = reader.size();
	if (originalSize.isValid() && !originalSize.isEmpty()) {
		reader.setScaledSize(originalSize.scaled(ThumbnailWidth, ThumbnailHeight, Qt::KeepAspectRatio));
	}

	const auto image = reader.read();
	if (image.isNull()) {
		return QIcon::fromTheme(QLatin1String("image-missing"));
	}

	return QIcon(QPixmap::fromImage(image));
}
}

CaptureHistoryDialog::CaptureHistoryDialog(const QSharedPointer<IRecentImageService> &recentImageService, QWidget *parent) :
	QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	mRecentImageService(recentImageService),
	mListWidget(new QListWidget(this)),
	mOpenButton(new QPushButton(tr("Open"), this)),
	mSaveAsButton(new QPushButton(tr("Save As..."), this)),
	mDeleteButton(new QPushButton(tr("Delete"), this))
{
	setWindowTitle(tr("Capture History"));
	setMinimumSize(640, 480);

	initGui();
	populateList();

	connect(mListWidget, &QListWidget::itemDoubleClicked, this, &CaptureHistoryDialog::openSelected);
	connect(mListWidget, &QListWidget::itemSelectionChanged, this, &CaptureHistoryDialog::updateButtons);
	connect(mOpenButton, &QPushButton::clicked, this, &CaptureHistoryDialog::openSelected);
	connect(mSaveAsButton, &QPushButton::clicked, this, &CaptureHistoryDialog::saveAsSelected);
	connect(mDeleteButton, &QPushButton::clicked, this, &CaptureHistoryDialog::deleteSelected);
}

void CaptureHistoryDialog::initGui()
{
	mListWidget->setViewMode(QListView::IconMode);
	mListWidget->setIconSize(QSize(ThumbnailWidth, ThumbnailHeight));
	mListWidget->setGridSize(QSize(ThumbnailWidth + 24, ThumbnailHeight + 44));
	mListWidget->setResizeMode(QListView::Adjust);
	mListWidget->setMovement(QListView::Static);
	mListWidget->setWordWrap(true);
	mListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	auto closeButton = new QPushButton(tr("Close"), this);
	connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

	auto buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(mOpenButton);
	buttonLayout->addWidget(mSaveAsButton);
	buttonLayout->addWidget(mDeleteButton);
	buttonLayout->addStretch();
	buttonLayout->addWidget(closeButton);

	auto mainLayout = new QVBoxLayout;
	mainLayout->addWidget(mListWidget);
	mainLayout->addLayout(buttonLayout);
	setLayout(mainLayout);

	updateButtons();
}

void CaptureHistoryDialog::populateList()
{
	mListWidget->clear();

	const auto history = mRecentImageService->getHistoryImagesPath();
	for (const auto &path : history) {
		if (!QFileInfo::exists(path)) {
			continue;
		}

		auto item = new QListWidgetItem(createThumbnail(path), QFileInfo(path).fileName());
		item->setToolTip(path);
		item->setData(Qt::UserRole, path);
		mListWidget->addItem(item);
	}

	updateButtons();
}

void CaptureHistoryDialog::refresh()
{
	populateList();
}

void CaptureHistoryDialog::updateButtons() const
{
	const auto hasSelection = !selectedPath().isEmpty();
	mOpenButton->setEnabled(hasSelection);
	mSaveAsButton->setEnabled(hasSelection);
	mDeleteButton->setEnabled(hasSelection);
}

QString CaptureHistoryDialog::selectedPath() const
{
	auto item = mListWidget->currentItem();
	if (item == nullptr || !item->isSelected()) {
		return QString();
	}
	return item->data(Qt::UserRole).toString();
}

void CaptureHistoryDialog::openSelected()
{
	const auto path = selectedPath();
	if (path.isEmpty()) {
		return;
	}

	emit openRequested(path);
	accept();
}

void CaptureHistoryDialog::saveAsSelected()
{
	const auto path = selectedPath();
	if (path.isEmpty()) {
		return;
	}

	const auto title = tr("Save Image As");
	const auto filter = tr("Image Files") + FileDialogFilterHelper::ImageFilesExport() + tr("All Files") + FileDialogFilterHelper::AllFiles();
	const auto targetPath = QFileDialog::getSaveFileName(this, title, QFileInfo(path).fileName(), filter);
	if (targetPath.isEmpty() || targetPath == path) {
		return;
	}

	if (QFileInfo::exists(targetPath)) {
		QFile::remove(targetPath);
	}
	QFile::copy(path, targetPath);
}

void CaptureHistoryDialog::deleteSelected()
{
	const auto path = selectedPath();
	if (path.isEmpty()) {
		return;
	}

	QFile::remove(path);
	mRecentImageService->removeImagePath(path);
	refresh();
}
