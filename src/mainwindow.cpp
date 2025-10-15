#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

// A map to hold language names and their codes
const QMap<QString, QString> languages = {
    {"Russian", "ru"},
    {"English", "en"},
    {"German", "de"},
    {"French", "fr"},
    {"Spanish", "es"}
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  networkManager = new QNetworkAccessManager(this);
  connect(networkManager, &QNetworkAccessManager::finished, this,
          &MainWindow::onTranslationFinished);

  // Populate language boxes
  for (auto it = languages.constBegin(); it != languages.constEnd(); ++it) {
      ui->sourceLangComboBox->addItem(it.key(), it.value());
      ui->targetLangComboBox->addItem(it.key(), it.value());
  }

  // Set default languages
  ui->sourceLangComboBox->setCurrentText("Russian");
  ui->targetLangComboBox->setCurrentText("English");

  this->setStyleSheet(R"(
        QWidget {
            background-color: #2e2f30;
            color: #e0e0e0;
            font-family: "Segoe UI";
            font-size: 11pt;
        }
        QMainWindow {
            background-color: #202122;
        }
        QTextEdit {
            background-color: #3a3b3c;
            border: 1px solid #4a4b4c;
            border-radius: 4px;
            padding: 5px;
        }
        QPushButton {
            background-color: #4a4b4c;
            border: 1px solid #5a5b5c;
            padding: 6px;
            border-radius: 4px;
            min-height: 20px;
        }
        QPushButton:hover {
            background-color: #5a5b5c;
        }
        QPushButton:pressed {
            background-color: #3a3b3c;
        }
        QComboBox {
            background-color: #3a3b3c;
            border: 1px solid #4a4b4c;
            border-radius: 4px;
            padding: 5px;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox QAbstractItemView {
            background-color: #3a3b3c;
            border: 1px solid #4a4b4c;
            selection-background-color: #4a4b4c;
        }
        QStatusBar {
            background-color: #202122;
        }
        QMenuBar {
            background-color: #2e2f30;
        }
        QMenuBar::item:selected {
            background-color: #4a4b4c;
        }
    )");
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_translateButton_clicked() {
  QString sourceText = ui->sourceTextEdit->toPlainText().trimmed();
  if (sourceText.isEmpty()) {
    return;
  }

  QString sourceLang = ui->sourceLangComboBox->currentData().toString();
  QString targetLang = ui->targetLangComboBox->currentData().toString();
  
  if (sourceLang == targetLang) {
      ui->translatedTextEdit->setText(sourceText);
      return;
  }

  QUrl url("https://api.mymemory.translated.net/get");
  QUrlQuery query;
  query.addQueryItem("q", sourceText);
  query.addQueryItem("langpair", QString("%1|%2").arg(sourceLang, targetLang));
  url.setQuery(query);

  QNetworkRequest request(url);
  networkManager->get(request);
}

void MainWindow::onTranslationFinished(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObj = jsonDoc.object();

    if (jsonObj.contains("responseData")) {
      QJsonObject responseDataObj = jsonObj["responseData"].toObject();
      if (responseDataObj.contains("translatedText")) {
        QString translatedText = responseDataObj["translatedText"].toString();
        ui->translatedTextEdit->setText(translatedText);
      }
    }
  } else {
    ui->translatedTextEdit->setText("Ошибка: " + reply->errorString());
  }
  reply->deleteLater();
}

void MainWindow::on_swapLangsButton_clicked()
{
    int sourceIndex = ui->sourceLangComboBox->currentIndex();
    ui->sourceLangComboBox->setCurrentIndex(ui->targetLangComboBox->currentIndex());
    ui->targetLangComboBox->setCurrentIndex(sourceIndex);
}
