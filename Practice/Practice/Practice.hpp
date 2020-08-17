#pragma once

#include <QtWidgets/QMainWindow>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QNetworkAccessManager>

#include <vector>
#include <map>

#include "ui_Practice.h"

class Practice : public QMainWindow
{
    Q_OBJECT

private:
    void _csv2chart(const QString& csvstr);
    void _json2chart(const QString& jsonstr);

    [[nodiscard]]
    auto _get_converted(const size_t val_idx) const;

    template<typename _SeriesType>
    void _draw_series(const QString& param_name, const std::vector<std::pair<qreal, qreal>>& values) const;

    void _draw(const QString& param_name, const std::vector<std::pair<qreal, qreal>>& values) const;

    void _show_warning(const std::string_view msg) const;

    void _update();
    void _update_pte_data(const QString& content);
    void _update_combo_params(const QString& uName);
    void _update_combo_source(const QString& uName);

    void _setup_chart();

    __forceinline void _add_log_msg(const QString& msg) const { m_ui->pte_log->appendPlainText(msg); }
    
private slots:
    void _on_net_result(QNetworkReply* reply);

    void _on_action_open_data();
    void _on_action_download_data();

public:
    Practice(QWidget* parent = nullptr);
    ~Practice() noexcept;

private:
    mutable Ui::PracticeClass                   *m_ui;
    QtCharts::QChart                            *m_chart;
    QNetworkAccessManager                       *m_net_manager;
    std::map<QString, std::vector<QStringList>>  m_data; // source(kinda device) -> [params_names, dates, values...]
};

template<typename _SeriesType>
inline void Practice::_draw_series(const QString& param_name, const std::vector<std::pair<qreal, qreal>>& values) const
{
    _add_log_msg("Drawing");

    m_ui->label_status->setText("Preparing data for drawing");

    auto bar = new QtCharts::QBarSet("");
    auto series = new _SeriesType;
    series->setUseOpenGL(true);
    series->setName(param_name);
    for (size_t i{}; i < values.size(); ++i)
    {
        auto&& [x, y] = values[i];

        if constexpr (std::is_same_v<_SeriesType, QtCharts::QBarSeries>)
            *bar << y;
        else if constexpr (std::is_base_of_v<QtCharts::QXYSeries, _SeriesType>)
            series->append(x, y);

        m_ui->progress_bar->setValue((i + 1LLU) * 1. / values.size() * 100.);
    }

    if constexpr (std::is_same_v<QtCharts::QScatterSeries, _SeriesType>)
    {
        series->setMarkerSize(5.);
        delete bar;
    }
    else if constexpr (std::is_same_v<_SeriesType, QtCharts::QBarSeries>)
        series->append(bar);

    m_chart->addSeries(series);   
    m_chart->createDefaultAxes();
    if constexpr (std::is_base_of_v<QtCharts::QXYSeries, _SeriesType>)
    {
        dynamic_cast<QtCharts::QValueAxis*>(m_chart->axisX())->setTickCount(std::max_element(values.begin(), values.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; })->first + 1);

        dynamic_cast<QtCharts::QValueAxis*>(m_chart->axisX())->setLabelFormat("%d");
    }
  
    series->attachAxis(m_chart->axisX());
    series->attachAxis(m_chart->axisY());

    m_ui->label_status->setText("OK");
}