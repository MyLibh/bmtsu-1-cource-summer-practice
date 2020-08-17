#include "Practice.hpp"

#include <QtCharts/QValueAxis>
#include <QNetworkReply>
#include <QMessageBox.h>

#include <algorithm>

#include "DownloadDialog.hpp"

namespace detail
{
    static constexpr auto DATE_FMT{ "yyyy-MM-dd hh:mm:ss" };

    typedef QtCharts::QAbstractAxis* (QtCharts::QChart::*axis_func_t)(QtCharts::QAbstractSeries*)const;

    void set_range(const qreal val, QtCharts::QChart* chart, axis_func_t axis)
    {
        auto min = std::min(val, dynamic_cast<QtCharts::QValueAxis*>((chart->*axis)(nullptr))->min());
        auto max = std::max(val, dynamic_cast<QtCharts::QValueAxis*>((chart->*axis)(nullptr))->max());
        (chart->*axis)(nullptr)->setRange(min, max);
    }

    auto as_is(const QStringList& dates, const QStringList& values)
    {
        static constexpr auto MINUTES_PER_DAY{ 1440. };

        std::vector<std::pair<qreal, qreal>> res;
        QDateTime start_dt;
        for (size_t i{}; i < values.size(); ++i)
        {
            auto dt = QDateTime::fromString(dates[i], DATE_FMT);
            qreal day_part = (dt.time().hour() * 60. + dt.time().minute()) / MINUTES_PER_DAY;

            if (!i)
            {
                start_dt = dt;
                start_dt.setTime(QTime(0, 0));
            }
       
            res.push_back({ start_dt.daysTo(QDateTime::fromString(dates[i], DATE_FMT)) + day_part, values[i].toDouble() });
        }

        return res;
    }

    auto average_hour(const QStringList& dates, const QStringList& values)
    {
        std::vector<std::pair<qreal, qreal>> res;
        size_t cur_hour{ std::numeric_limits<size_t>::max() };
        QDateTime start_dt;
        qreal sum{};
        size_t num{ 1 };
        for (size_t i{}; i < values.size(); ++i)
        {  
            size_t hour = QStringRef(&dates[i], 10, 3).toInt();

            if (i)
            {
                if (hour != cur_hour || i + 1 == values.size())
                {
                    auto dt = QDateTime::fromString(dates[i - 1], DATE_FMT);
                    
                    res.push_back({ start_dt.daysTo(dt) + cur_hour / 24., sum / num });
                    
                    cur_hour = hour;
                    num = 1;
                    sum = values[i].toDouble();
                }
                else
                    num++, sum += values[i].toDouble();
            }
            else
            {
                start_dt = QDateTime::fromString(dates[i], DATE_FMT);
                start_dt.setTime(QTime(0, 0));

                cur_hour = hour;
                sum += values[i].toDouble();
            }
        }

        return res;
    }

    auto average_3hour(const QStringList& dates, const QStringList& values)
    {
        std::vector<std::pair<qreal, qreal>> res;
        size_t cur_hour{ std::numeric_limits<size_t>::max() };
        QDateTime start_dt;
        QDateTime cur_dt;
        qreal sum{};
        size_t num{ 1 };
        for (size_t i{}; i < values.size(); ++i)
        {
            size_t hour = QStringRef(&dates[i], 10, 3).toInt();

            if (i)
            {
                if (!(cur_hour == hour || (cur_hour + 1) % 24 == hour || (cur_hour + 2) % 24 == hour) || i + 1 == values.size())
                {
                    auto dt = QDateTime::fromString(dates[i], DATE_FMT);

                    res.push_back({ start_dt.daysTo(cur_dt) + cur_hour / 24., sum / num });

                    cur_dt = dt;
                    cur_hour = hour;
                    num = 1;
                    sum = values[i].toDouble();
                }
                else
                    num++, sum += values[i].toDouble();
            }
            else
            {
                start_dt = QDateTime::fromString(dates[i], DATE_FMT);
                start_dt.setTime(QTime(0, 0));

                cur_dt = start_dt;
                cur_hour = hour;
                sum += values[i].toDouble();
            }
        }

        return res;
    }

    auto average_day(const QStringList& dates, const QStringList& values)
    {
        std::vector<std::pair<qreal, qreal>> res;
        QDateTime start_dt;
        size_t cur_day{};
        qreal sum{};
        size_t num{ 1 };
        for (size_t i{}; i < values.size(); ++i)
        {
            size_t day = QStringRef(&dates[i], 8, 2).toInt();

            if (i)
            {
                if (day != cur_day || i + 1 == values.size())
                {
                    if (i + 1 == values.size())
                    {
                        num++;
                        sum += values[i].toDouble();
                    }

                    auto dt = QDateTime::fromString(dates[i - 1], DATE_FMT);

                    res.push_back({ start_dt.daysTo(dt), sum / num });
              
                    cur_day = day;
                    num = 1;
                    sum = values[i].toDouble();
                }
                else
                    num++, sum += values[i].toDouble();
            }
            else
            {
                start_dt = QDateTime::fromString(dates[i], DATE_FMT);
                start_dt.setTime(QTime(0, 0));
                cur_day = day;
                sum += values[i].toDouble();
            }
        }

        return res;
    }

    auto min_day(const QStringList& dates, const QStringList& values)
    {
        std::vector<std::pair<qreal, qreal>> res;
        QDateTime start_dt;
        size_t cur_day{};
        qreal min{ std::numeric_limits<qreal>::max() };
        for (size_t i{}; i < values.size(); ++i)
        {
            size_t day = QStringRef(&dates[i], 8, 2).toInt();

            if (i)
            {
                if (day != cur_day || i + 1 == values.size())
                {
                    if (i + 1 == values.size())
                        min = std::min(min, values[i].toDouble());

                    auto dt = QDateTime::fromString(dates[i - 1], DATE_FMT);

                    res.push_back({ start_dt.daysTo(dt), min });

                    cur_day = day;
                    min = values[i].toDouble();
                }
                else
                    min = std::min(min, values[i].toDouble());
            }
            else
            {
                start_dt = QDateTime::fromString(dates[i], DATE_FMT);
                start_dt.setTime(QTime(0, 0));

                cur_day = day;
                min = values[i].toDouble();
            }
        }

        return res;
    }

    auto max_day(const QStringList& dates, const QStringList& values)
    {
        std::vector<std::pair<qreal, qreal>> res;
        QDateTime start_dt;
        size_t cur_day{};
        qreal max{ std::numeric_limits<qreal>::min() };
        for (size_t i{}; i < values.size(); ++i)
        {
            size_t day = QStringRef(&dates[i], 8, 2).toInt();

            if (i)
            {
                if (day != cur_day || i + 1 == values.size())
                {
                    if (i + 1 == values.size())
                        max = std::max(max, values[i].toDouble());

                    auto dt = QDateTime::fromString(dates[i - 1], DATE_FMT);

                    res.push_back({ start_dt.daysTo(dt), max });

                    cur_day = day;
                    max = values[i].toDouble();
                }
                else
                    max = std::max(max, values[i].toDouble());
            }
            else
            {
                start_dt = QDateTime::fromString(dates[i], DATE_FMT);
                start_dt.setTime(QTime(0, 0));
                cur_day = day;
                max = values[i].toDouble();
            }
        }

        return res;
    }
} // namespace detail

void Practice::_csv2chart(const QString& csvstr)
{
    _add_log_msg("Parsing csv");

    m_ui->label_status->setText("Parsing csv");

    QString uName = m_ui->combo_source->currentText();
    if (m_data.find(uName) != m_data.end())
        m_data[uName].clear();

    auto lines = csvstr.split("\n");

    m_data[uName].push_back(lines[1].split(";"));
    m_data[uName][0].removeFirst();

    _update_combo_params(uName);

    m_data[uName].resize(m_data[uName][0].size() + 2); // for DateTime and names
    for (size_t i{ 2 }; i < lines.size(); ++i)
    {
        auto values = lines[i].split(";");
        for (size_t j{}; j < values.size(); ++j)
            m_data[uName][j + 1].push_back(values[j]);
    }

    m_ui->label_status->setText("OK");
}

void Practice::_json2chart(const QString& jsonstr)
{
    _add_log_msg("Parsing json");

    m_ui->label_status->setText("Parsing json");

    QJsonDocument json_doc = QJsonDocument::fromJson(jsonstr.toUtf8());
    QJsonObject json = json_doc.object();

    size_t progress{};
    m_ui->progress_bar->setValue(0);

    QString title;
    bool first = true;
    for (const auto& i : json.keys())
    {
        QJsonValue value = json.value(QString(i));
        QJsonObject item = value.toObject();

        QString uName = item.value(QString("uName")).toString().toUtf8();

        if (title.isEmpty())
            title = uName;

        if (uName == title)
        {
            QJsonObject data = item.value(QString("data")).toObject();

            auto keys = data.keys();
            
            if (first)
            {
                _update_combo_source(uName);
                m_data[uName].clear();

                // keys.erase(std::remove_if(keys.begin(), keys.end(), [](const auto& s) { return s.startsWith("system"); }), keys.end());

                m_data[uName].resize(data.keys().size() + 2); // +1 date; +1 names

                m_data[uName][0] = keys;
                _update_combo_params(uName);

                first = false;
            }
            
            m_data[uName][1].append(item.value(QString("Date")).toString());

            size_t idx{ 2 }; // pass DateTime and names
            for (const auto& param_name : keys)
                m_data[uName][idx++].append(data.value(QString(param_name)).toString());

            m_ui->progress_bar->setValue((1. + progress++) / json.keys().size() * 100);
        }
    }

    m_ui->label_status->setText("OK");
}

[[nodiscard]]
auto Practice::_get_converted(const size_t val_idx) const
{
    _add_log_msg("Applying convesations");

    m_ui->label_status->setText("Calculating");

    using func_t = std::function<decltype(detail::as_is)>;
    static func_t funcs[]
    {
        &detail::as_is,
        &detail::average_hour,
        &detail::average_3hour,
        &detail::average_day,
        &detail::min_day,
        &detail::max_day,
    };

    m_chart->axisX()->setTitleText("t, day");

    QString uName = m_ui->combo_source->currentText();
    auto res = funcs[m_ui->combo_avg->currentIndex()](m_data.at(uName)[1], m_data.at(uName)[val_idx + 2]);

    m_ui->label_status->setText("OK");

    return res;
}

void Practice::_draw(const QString& param_name, const std::vector<std::pair<qreal, qreal>>& values) const
{
    if (auto idx = m_ui->combo_graph_type->currentIndex(); !idx)
        _draw_series<QtCharts::QLineSeries>(param_name, values);
    else if (idx == 1)
        _draw_series<QtCharts::QBarSeries>(param_name, values);
    else
        _draw_series<QtCharts::QScatterSeries>(param_name, values);
}

void Practice::_update()
{
    if (m_data.empty())
    {
        _show_warning("No data");

        return;
    }

    size_t val_idx = m_ui->combo_params->currentIndex();
    QString param_name = m_data[m_ui->combo_source->currentText()][0][val_idx];
    m_chart->axisY()->setTitleText(param_name);

    auto res = _get_converted(val_idx);
    
    _draw(param_name, res);
}

void Practice::_show_warning(const std::string_view msg) const
{
    _add_log_msg(QString("[WARNING]: ") + msg.data());

    QMessageBox _(QMessageBox::Icon::Warning,
        "Warning",
        msg.data(),
        QMessageBox::StandardButton::Ok);

    _.exec();
}

void Practice::_update_pte_data(const QString& content)
{
    static constexpr auto MAX_LENGTH{ 5000U };

    if (content.size() > MAX_LENGTH)
    {
        QString small_content = content.mid(0, MAX_LENGTH);

        small_content.append("\n\n...");

        m_ui->pte_data->document()->setPlainText(small_content);
    }
    else
        m_ui->pte_data->document()->setPlainText(content);
}

void Practice::_update_combo_params(const QString& uName)
{
    _add_log_msg("Updating params");

    m_ui->combo_params->clear();
    m_ui->combo_params->addItems(m_data[uName][0]);
}

void Practice::_update_combo_source(const QString& uName)
{
    _add_log_msg("Updating sources");

    m_ui->combo_source->addItem(uName);
    m_ui->combo_source->setCurrentText(uName);
}

void Practice::_setup_chart()
{
    _add_log_msg("Setting up chart");

    m_chart->addAxis(new QtCharts::QValueAxis, Qt::AlignBottom);
    m_chart->addAxis(new QtCharts::QValueAxis, Qt::AlignLeft);

    m_ui->graphic->setChart(m_chart);
    m_ui->graphic->setRenderHint(QPainter::Antialiasing);
}

void Practice::_on_net_result(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        reply->ignoreSslErrors();

        _show_warning("SSL error occurred");
    }
    else
    {
        QString content = reply->readAll();
        _update_pte_data(content);

        m_ui->progress_bar->setValue(50);

        _json2chart(content);

        m_ui->progress_bar->setValue(100);
    }

    reply->deleteLater();
}

void Practice::_on_action_open_data()
{
    _add_log_msg("Opening file");

    QString filepath = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("CSV(*.csv);;JSON(*.json)"));
    if (filepath.isEmpty())
    {
        _add_log_msg("Canceled");

        return;
    }

    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        _show_warning("Failed to open file");
    
        return;
    }
    
    QString content = file.readAll();
    file.close();

    _update_pte_data(content);
    
    QString uName = QFileInfo(filepath).fileName().section(".", 0, 0);
    
    if (filepath.endsWith("csv"))
    {
        _update_combo_source(uName);
        _csv2chart(content);
    }
    else
        _json2chart(content);
}

void Practice::_on_action_download_data()
{
    static const std::pair<QString, QString> NAME_SERIAL[]
    {
        { "Тест%20Студии", "schHome" },
        { "РОСА%20К-2", "01" }
    };

    DownloadDialog dialog(this);

    dialog.exec();

    auto&& [from, to, idx] = dialog.get_values();
    auto&& [uName, serial] = NAME_SERIAL[idx];
    QString url = "http://webrobo.mgul.ac.ru:3000/db_api_REST/calibr/log/" + from + "/" + to + "/" + uName + "/" + serial;

    _add_log_msg("Downloading");

    QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
    ssl.setProtocol(QSsl::AnyProtocol);

    QSslConfiguration::setDefaultConfiguration(ssl);

    QNetworkRequest req;
    req.setUrl(QUrl(url));
    req.setSslConfiguration(ssl);

    m_ui->label_status->setText("Downloading");
    m_ui->progress_bar->setValue(0);
    m_net_manager->get(req);
}

Practice::Practice(QWidget* parent /* = nullptr */) :
    QMainWindow(parent),
    m_ui(new Ui::PracticeClass),
    m_chart(new QtCharts::QChart),
    m_net_manager(nullptr),
    m_data()
{
    m_ui->setupUi(this);

    _add_log_msg("Setting up");

    _setup_chart();

    m_net_manager = new QNetworkAccessManager(this);
    QObject::connect(m_net_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(_on_net_result(QNetworkReply*)));

    QObject::connect(m_ui->action_open_data, &QAction::triggered, this, &Practice::_on_action_open_data);
    QObject::connect(m_ui->action_download_data, &QAction::triggered, this, &Practice::_on_action_download_data);

    QObject::connect(m_ui->clear_button, &QPushButton::pressed, [this]() { m_chart->removeAllSeries(); m_chart->setTitle(""); });
    QObject::connect(m_ui->add_series_button, &QPushButton::pressed, this, &Practice::_update);

    using pfunc_t = void(QComboBox::*)(const QString&);
    QObject::connect(m_ui->combo_source, pfunc_t{ &QComboBox::activated }, [this]() { _update_combo_params(m_ui->combo_source->currentText()); });

    _add_log_msg("Setup finished");
}

Practice::~Practice() noexcept
{
    m_chart->removeAllSeries();

    delete m_net_manager;
    delete m_chart;
    delete m_ui;
}