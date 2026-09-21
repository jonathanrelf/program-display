// SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "program_display_dock.hpp"

#include <QColor>
#include <QComboBox>
#include <QEvent>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QStandardItemModel>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>

namespace program_display {
namespace {

QLabel *valueLabel(QWidget *parent)
{
	auto *label = new QLabel(parent);
	label->setMinimumWidth(0);
	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	label->setTextInteractionFlags(Qt::TextSelectableByMouse);
	label->setWordWrap(true);
	return label;
}

QLabel *fieldLabel(const QString &text, QWidget *parent)
{
	auto *label = new QLabel(text, parent);
	label->setForegroundRole(QPalette::PlaceholderText);
	label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	return label;
}

QColor mix(const QColor &base, const QColor &accent, int accent_percent)
{
	const int base_percent = 100 - accent_percent;
	return {((base.red() * base_percent) + (accent.red() * accent_percent)) / 100,
		((base.green() * base_percent) + (accent.green() * accent_percent)) / 100,
		((base.blue() * base_percent) + (accent.blue() * accent_percent)) / 100};
}

} // namespace

ProgramDisplayDock::ProgramDisplayDock(QWidget *parent) : QWidget(parent)
{
	auto *layout = new QVBoxLayout(this);
	layout->setContentsMargins(12, 12, 12, 12);
	layout->setSpacing(10);
	setMinimumWidth(320);

	auto *summary = new QFrame(this);
	summary->setObjectName(QStringLiteral("programDisplaySummary"));
	summary->setFrameShape(QFrame::StyledPanel);
	summary->setStyleSheet(
		QStringLiteral("QFrame#programDisplaySummary { background: palette(alternate-base); border: 1px solid "
			       "palette(mid); border-radius: 6px; }"));
	auto *summary_layout = new QVBoxLayout(summary);
	summary_layout->setContentsMargins(10, 9, 10, 9);
	summary_layout->setSpacing(5);

	auto *summary_header = new QHBoxLayout();
	summary_header->setSpacing(8);
	auto *title = new QLabel(tr("Program display"), summary);
	QFont title_font = title->font();
	title_font.setBold(true);
	title->setFont(title_font);
	summary_header->addWidget(title);
	summary_header->addStretch();

	status_value_ = new QLabel(summary);
	status_value_->setAccessibleName(tr("Program output status"));
	status_value_->setAlignment(Qt::AlignCenter);
	summary_header->addWidget(status_value_);
	summary_layout->addLayout(summary_header);

	reason_value_ = valueLabel(summary);
	reason_value_->setAccessibleName(tr("Program output status reason"));
	summary_layout->addWidget(reason_value_);
	layout->addWidget(summary);

	auto *destination_label = new QLabel(tr("Destination"), this);
	QFont destination_font = destination_label->font();
	destination_font.setBold(true);
	destination_label->setFont(destination_font);
	layout->addWidget(destination_label);

	destination_layout_ = new QGridLayout();
	destination_layout_->setContentsMargins(0, 0, 0, 0);
	destination_layout_->setHorizontalSpacing(8);
	destination_layout_->setVerticalSpacing(8);

	destination_combo_ = new QComboBox(this);
	destination_combo_->setAccessibleName(tr("Program destination"));
	destination_combo_->setMinimumContentsLength(14);
	destination_combo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	destination_combo_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	destination_layout_->addWidget(destination_combo_, 0, 0);

	start_button_ = new QPushButton(tr("Start output"), this);
	start_button_->setAccessibleName(tr("Start Program output"));
	destination_layout_->addWidget(start_button_, 0, 1);
	destination_layout_->setColumnStretch(0, 1);
	layout->addLayout(destination_layout_);

	details_button_ = new QToolButton(this);
	details_button_->setText(tr("Details"));
	details_button_->setCheckable(true);
	details_button_->setArrowType(Qt::RightArrow);
	details_button_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	details_button_->setAccessibleName(tr("Show output details"));
	layout->addWidget(details_button_, 0, Qt::AlignLeft);

	details_scroll_ = new QScrollArea(this);
	details_scroll_->setFrameShape(QFrame::NoFrame);
	details_scroll_->setWidgetResizable(true);
	details_scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	details_scroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	details_scroll_->setMinimumHeight(90);
	details_scroll_->setMaximumHeight(180);
	details_scroll_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	details_panel_ = new QWidget();
	auto *form = new QFormLayout(details_panel_);
	form->setContentsMargins(0, 0, 0, 0);
	form->setHorizontalSpacing(14);
	form->setVerticalSpacing(6);
	form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	form->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
	form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

	configured_value_ = valueLabel(details_panel_);
	resolved_value_ = valueLabel(details_panel_);
	scene_value_ = valueLabel(details_panel_);
	video_value_ = valueLabel(details_panel_);
	audio_value_ = valueLabel(details_panel_);

	form->addRow(fieldLabel(tr("Configured"), details_panel_), configured_value_);
	form->addRow(fieldLabel(tr("Resolved"), details_panel_), resolved_value_);
	form->addRow(fieldLabel(tr("Scene"), details_panel_), scene_value_);
	form->addRow(fieldLabel(tr("Video"), details_panel_), video_value_);
	form->addRow(fieldLabel(tr("Audio"), details_panel_), audio_value_);
	details_scroll_->setWidget(details_panel_);
	details_scroll_->hide();
	layout->addWidget(details_scroll_, 1);
	layout->addStretch();

	connect(start_button_, &QPushButton::clicked, this, [this]() {
		const int index = destination_combo_->currentIndex();
		if (index < 0 || index >= static_cast<int>(destination_screens_.size()) ||
		    !destination_selectable_[static_cast<std::size_t>(index)] || !start_handler_) {
			return;
		}
		if (QScreen *screen = destination_screens_[static_cast<std::size_t>(index)]) {
			start_handler_(screen);
		}
	});
	connect(destination_combo_, &QComboBox::currentIndexChanged, this, [this](int index) {
		if (index >= 0 && index < static_cast<int>(destination_descriptions_.size())) {
			destination_combo_->setToolTip(
				QString::fromStdString(destination_descriptions_[static_cast<std::size_t>(index)]));
		} else {
			destination_combo_->setToolTip({});
		}
		updateStartButton();
	});
	connect(details_button_, &QToolButton::toggled, this, [this](bool expanded) {
		details_scroll_->setVisible(expanded);
		details_button_->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
		details_button_->setAccessibleName(expanded ? tr("Hide output details") : tr("Show output details"));
	});
}

void ProgramDisplayDock::setDestinations(const std::vector<DestinationOption> &destinations, QScreen *selected)
{
	int selected_index = -1;
	{
		const QSignalBlocker blocker(destination_combo_);
		destination_combo_->clear();
		destination_screens_.clear();
		destination_selectable_.clear();
		destination_association_required_.clear();
		destination_descriptions_.clear();

		for (const auto &destination : destinations) {
			const int index = destination_combo_->count();
			destination_combo_->addItem(QString::fromStdString(destination.label));
			destination_combo_->setItemData(index, QString::fromStdString(destination.description),
							Qt::ToolTipRole);
			destination_combo_->setItemData(index, QString::fromStdString(destination.description),
							Qt::AccessibleDescriptionRole);
			destination_screens_.push_back(destination.screen);
			destination_selectable_.push_back(destination.selectable);
			destination_association_required_.push_back(destination.association_required);
			destination_descriptions_.push_back(destination.description);
			if (!destination.selectable) {
				auto *model = qobject_cast<QStandardItemModel *>(destination_combo_->model());
				if (model && model->item(index)) {
					model->item(index)->setEnabled(false);
				}
			}
			if (destination.screen == selected) {
				selected_index = index;
			}
		}

		if (selected_index >= 0) {
			destination_combo_->setCurrentIndex(selected_index);
		} else if (!destinations.empty()) {
			destination_combo_->setCurrentIndex(0);
		}
	}

	const int index = destination_combo_->currentIndex();
	if (index >= 0 && index < static_cast<int>(destination_descriptions_.size())) {
		destination_combo_->setToolTip(
			QString::fromStdString(destination_descriptions_[static_cast<std::size_t>(index)]));
	} else {
		destination_combo_->setToolTip({});
	}
	updateStartButton();
}

void ProgramDisplayDock::setViewModel(const DockViewModel &view_model)
{
	current_status_ = view_model.status;
	request_outstanding_ = view_model.request_outstanding;
	QString visible_status = QString::fromStdString(view_model.status);
	if (request_outstanding_) {
		visible_status = tr("OUTPUT REQUESTED");
	} else if (view_model.status == "UNCONFIGURED") {
		visible_status = tr("NOT CONFIGURED");
	} else if (view_model.status == "MISSING") {
		visible_status = tr("DISPLAY MISSING");
	}
	status_value_->setText(visible_status);
	updateStatusAppearance();
	reason_value_->setText(QString::fromStdString(view_model.reason));
	configured_value_->setText(QString::fromStdString(view_model.configured_destination));
	resolved_value_->setText(QString::fromStdString(view_model.resolved_destination));
	scene_value_->setText(QString::fromStdString(view_model.program_scene));
	video_value_->setText(QString::fromStdString(view_model.video_format));
	audio_value_->setText(QString::fromStdString(view_model.audio_summary));
	controls_enabled_ = view_model.controls_enabled;
	destination_combo_->setEnabled(controls_enabled_);
	updateStartButton();
}

void ProgramDisplayDock::changeEvent(QEvent *event)
{
	QWidget::changeEvent(event);
	if (event->type() == QEvent::PaletteChange) {
		updateStatusAppearance();
	}
}

void ProgramDisplayDock::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	updateDestinationLayout();
}

QSize ProgramDisplayDock::sizeHint() const
{
	QSize preferred = QWidget::sizeHint();
	preferred.setWidth(std::max(preferred.width(), 400));
	return preferred;
}

void ProgramDisplayDock::updateDestinationLayout()
{
	constexpr int kStackDestinationBelowWidth = 380;
	const bool stacked = width() < kStackDestinationBelowWidth;
	if (stacked == destination_stacked_) {
		return;
	}

	destination_stacked_ = stacked;
	destination_layout_->removeWidget(destination_combo_);
	destination_layout_->removeWidget(start_button_);
	if (stacked) {
		destination_layout_->addWidget(destination_combo_, 0, 0);
		destination_layout_->addWidget(start_button_, 1, 0);
	} else {
		destination_layout_->addWidget(destination_combo_, 0, 0);
		destination_layout_->addWidget(start_button_, 0, 1);
	}
	destination_layout_->setColumnStretch(0, 1);
	destination_layout_->setColumnStretch(1, 0);
}

void ProgramDisplayDock::updateStartButton()
{
	const int index = destination_combo_->currentIndex();
	const bool has_destination = index >= 0 && index < static_cast<int>(destination_selectable_.size());
	const bool selectable = has_destination && destination_selectable_[static_cast<std::size_t>(index)];
	const bool association_required = has_destination &&
					  index < static_cast<int>(destination_association_required_.size()) &&
					  destination_association_required_[static_cast<std::size_t>(index)];

	if (!controls_enabled_ && current_status_ == "STARTING") {
		start_button_->setText(tr("Starting…"));
		start_button_->setToolTip(tr("Waiting for OBS to finish loading."));
	} else if (!controls_enabled_ && request_outstanding_) {
		start_button_->setText(tr("Output requested"));
		start_button_->setToolTip(
			tr("Program should be visible on the selected display. OBS cannot confirm that it remains open. "
			   "Restart OBS before changing destination."));
	} else if (!has_destination) {
		start_button_->setText(tr("No display"));
		start_button_->setToolTip(tr("Connect a display to choose a Program destination."));
	} else if (!selectable && current_status_ == "MISSING") {
		start_button_->setText(tr("Display missing"));
		start_button_->setToolTip(
			tr("Reconnect the saved display, or deliberately choose another destination."));
	} else if (!selectable) {
		start_button_->setText(tr("Identity unavailable"));
		start_button_->setToolTip(
			tr("This display is connected but does not expose enough identity metadata for safe "
			   "restoration."));
	} else if (association_required) {
		start_button_->setText(tr("Remember && start"));
		start_button_->setToolTip(tr("Remember this serial-less display profile and start Program output."));
	} else {
		start_button_->setText(tr("Start output"));
		start_button_->setToolTip(tr("Open OBS Program on this destination."));
	}
	start_button_->setEnabled(controls_enabled_ && selectable);
}

void ProgramDisplayDock::updateStatusAppearance()
{
	QColor accent(123, 132, 148);
	if (request_outstanding_) {
		accent = QColor(65, 139, 230);
	} else if (current_status_ == "STARTING") {
		accent = QColor(65, 139, 230);
	} else if (current_status_ == "DEGRADED") {
		accent = QColor(222, 159, 55);
	} else if (current_status_ == "MISSING") {
		accent = QColor(224, 112, 54);
	} else if (current_status_ == "ERROR") {
		accent = QColor(220, 72, 72);
	} else if (!current_status_.empty() && current_status_ != "UNCONFIGURED") {
		accent = QColor(66, 170, 103);
	}

	const QColor window = palette().color(QPalette::Window);
	const bool dark = window.lightness() < 128;
	const QColor background = mix(window, accent, dark ? 28 : 14);
	const QColor foreground = dark ? accent.lighter(145) : accent.darker(155);
	const QColor border = mix(window, accent, dark ? 62 : 48);
	status_value_->setStyleSheet(QStringLiteral("QLabel { background: %1; color: %2; border: 1px solid %3; "
						    "border-radius: 9px; padding: 2px 8px; font-weight: 600; }")
					     .arg(background.name(), foreground.name(), border.name()));
}

void ProgramDisplayDock::setStartHandler(std::function<void(QScreen *)> handler)
{
	start_handler_ = std::move(handler);
}

} // namespace program_display
