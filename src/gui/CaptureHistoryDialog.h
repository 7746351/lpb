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

#ifndef KSNIP_CAPTUREHISTORYDIALOG_H
#define KSNIP_CAPTUREHISTORYDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QSharedPointer>

#include "src/backend/recentImages/IRecentImageService.h"

class CaptureHistoryDialog : public QDialog
{
	Q_OBJECT
public:
	explicit CaptureHistoryDialog(const QSharedPointer<IRecentImageService> &recentImageService, QWidget *parent);
	~CaptureHistoryDialog() override = default;

signals:
	void openRequested(const QString &path);

private:
	QSharedPointer<IRecentImageService> mRecentImageService;
	QListWidget *mListWidget;
	QPushButton *mOpenButton;
	QPushButton *mSaveAsButton;
	QPushButton *mDeleteButton;

	void initGui();
	void populateList();
	void updateButtons() const;
	QString selectedPath() const;

private slots:
	void refresh();
	void openSelected();
	void saveAsSelected();
	void deleteSelected();
};

#endif //KSNIP_CAPTUREHISTORYDIALOG_H
