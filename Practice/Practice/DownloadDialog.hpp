#pragma once

#include <qdialog.h>

#include <tuple>

namespace Ui
{
	class Dialog;
} // namespace Ui

class DownloadDialog : public QDialog
{
	Q_OBJECT

public:
	using params_t = std::tuple<QString, QString, int>;

public:
	DownloadDialog(QWidget* parent = nullptr);
	inline ~DownloadDialog() noexcept { delete m_ui; };

	[[nodiscard]]
	params_t get_values() const noexcept;

private:
	Ui::Dialog *m_ui;
};