#include "DownloadDialog.hpp"

#include <qwidget.h>

#include "ui_DownloadDialog.h"

constexpr auto REST_API_DATE_FORMAT{ "yyyyMMddThhmmss" };

DownloadDialog::DownloadDialog(QWidget* parent /* = nullptr */) :
	QDialog(parent),
	m_ui(new Ui::Dialog)
{
	m_ui->setupUi(this);
	
	QObject::connect(m_ui->download_button, &QPushButton::pressed,
		[this]() { close(); });
}

[[nodiscard]]
DownloadDialog::params_t DownloadDialog::get_values() const noexcept { return { m_ui->dt_edit_from->dateTime().toString(REST_API_DATE_FORMAT), m_ui->dt_edit_to->dateTime().toString(REST_API_DATE_FORMAT), m_ui->combo_device->currentIndex() }; } // RVO