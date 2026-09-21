// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QPointer>
#include <QWidget>

#include <functional>
#include <string>
#include <vector>

class QComboBox;
class QEvent;
class QGridLayout;
class QLabel;
class QPushButton;
class QResizeEvent;
class QScreen;
class QScrollArea;
class QToolButton;

namespace program_display {

struct DestinationOption {
	QPointer<QScreen> screen;
	std::string label;
	std::string description;
	bool selectable = true;
	bool association_required = false;
};

struct DockViewModel {
	std::string status;
	std::string reason;
	std::string configured_destination;
	std::string resolved_destination;
	std::string program_scene;
	std::string video_format;
	std::string audio_summary;
	bool request_outstanding = false;
	bool controls_enabled = false;
};

class ProgramDisplayDock final : public QWidget {
public:
	explicit ProgramDisplayDock(QWidget *parent = nullptr);

	void setDestinations(const std::vector<DestinationOption> &destinations, QScreen *selected);
	void setViewModel(const DockViewModel &view_model);
	void setStartHandler(std::function<void(QScreen *)> handler);

private:
	void changeEvent(QEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;
	QSize sizeHint() const override;
	void updateDestinationLayout();
	void updateStartButton();
	void updateStatusAppearance();

	QGridLayout *destination_layout_ = nullptr;
	QComboBox *destination_combo_ = nullptr;
	QPushButton *start_button_ = nullptr;
	QToolButton *details_button_ = nullptr;
	QScrollArea *details_scroll_ = nullptr;
	QWidget *details_panel_ = nullptr;
	QLabel *status_value_ = nullptr;
	QLabel *reason_value_ = nullptr;
	QLabel *configured_value_ = nullptr;
	QLabel *resolved_value_ = nullptr;
	QLabel *scene_value_ = nullptr;
	QLabel *video_value_ = nullptr;
	QLabel *audio_value_ = nullptr;
	std::vector<QPointer<QScreen>> destination_screens_;
	std::vector<bool> destination_selectable_;
	std::vector<bool> destination_association_required_;
	std::vector<std::string> destination_descriptions_;
	std::function<void(QScreen *)> start_handler_;
	std::string current_status_;
	bool destination_stacked_ = false;
	bool request_outstanding_ = false;
	bool controls_enabled_ = false;
};

} // namespace program_display
