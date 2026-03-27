#include "../MessageArea/messagearea.h"
#include "../MessageArea/messagemodel.h"
#include "../../../../usermanager.h"
#include "../../../../Properties/sourcemanager.h"
#include "../../../../tcpmanager.h"
#include "../../../../database.h"
#include "inputarea.h"
#include <QPushButton>
#include <QDir>
#include <QDebug>
#include <QVBoxLayout>
#include <QMenu>
#include <QWidgetAction>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QKeyEvent>
#include <QTextBlock>
#include <QMimeDatabase>
#include <QApplication>
#include <QShortcut>
#include <QMimeData>
#include <QClipboard>
#include <QLabel>
#include <QAudioFormat>
#include <QAudioSource>
#include <QBuffer>
#include <QTimer>
#include <QMediaDevices>
#include <qstyle.h>
#include <QAudioSink>

InputArea::InputArea(QWidget *parent) : QWidget(parent)
{
    setupUI();
    setupConnections();
    setupEmojiMenu();
}

InputArea::~InputArea()
{
}

void InputArea::setupUI()
{
    setMinimumHeight(150);
    setMaximumHeight(400);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 顶部按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setAlignment(Qt::AlignLeft);
    buttonLayout->setContentsMargins(5,8,0,0);

    // 文件按钮
    m_fileButton = new QToolButton;
    m_fileButton->setObjectName("fileButton");
    m_fileButton->setToolTip("Files");
    m_fileButton->setIcon(QIcon(":/Resources/main/file.png"));
    m_fileButton->setIconSize({20,20});
    m_fileButton->setFixedSize({30,30});

    // emoji按钮
    m_emojiButton = new QToolButton;
    m_emojiButton->setObjectName("emojiButton");
    m_emojiButton->setToolTip("Emojis");
    m_emojiButton->setIcon(QIcon(":/Resources/main/emoji.png"));
    m_emojiButton->setIconSize({30,30});
    m_emojiButton->setFixedSize({30,30});
    m_emojiButton->setPopupMode(QToolButton::InstantPopup);
    m_emojiButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // 图片按钮
    m_imageButton = new QToolButton;
    m_imageButton->setObjectName("imageButton");
    m_imageButton->setToolTip("Pictures");
    m_imageButton->setIcon(QIcon(":/Resources/main/picture.png"));
    m_imageButton->setIconSize({25,25});
    m_imageButton->setFixedSize({30,30});

    // 视频按钮
    m_videoButton = new QToolButton;
    m_videoButton->setObjectName("videoButton");
    m_videoButton->setToolTip("Videos");
    m_videoButton->setIcon(QIcon(":/Resources/main/video.png"));
    m_videoButton->setIconSize({25,25});
    m_videoButton->setFixedSize({30,30});

    // 录音按钮
    m_audioButton = new QToolButton;
    m_audioButton->setObjectName("audioButton");
    m_audioButton->setToolTip("Audios");
    m_audioButton->setIcon(QIcon(":/Resources/main/audio.png"));
    m_audioButton->setIconSize({25,25});
    m_audioButton->setFixedSize({30,30});

    // 截屏按钮
    m_captureButton = new QToolButton;
    m_captureButton->setObjectName("captureButton");
    m_captureButton->setToolTip("Capture");
    m_captureButton->setIcon(QIcon(":/Resources/main/capture.png"));
    m_captureButton->setIconSize({25,25});
    m_captureButton->setFixedSize({30,30});

    buttonLayout->addWidget(m_fileButton);
    buttonLayout->addWidget(m_emojiButton);
    buttonLayout->addWidget(m_imageButton);
    buttonLayout->addWidget(m_videoButton);
    buttonLayout->addWidget(m_audioButton);
    buttonLayout->addWidget(m_captureButton);
    buttonLayout->setSpacing(8);

    // 输入框
    m_textEdit = new QTextEdit;
    m_textEdit->setPlaceholderText("Enter Message...");
    // 谁要被过滤事件，谁就安装过滤器，谁要管理过滤事件，就设置为过滤器的对象。
    // 这样，安装过滤器的对象的事件就会先发给过滤器对象处理
    // 被监控对象->installEventFilter(监控者);
    m_textEdit->installEventFilter(this);   // 事件被this过滤
    m_textEdit->setAcceptRichText(true);
    m_textEdit->setMinimumHeight(80);
    m_textEdit->setObjectName("textEdit");

    // 设置字体
    QFont font("Microsoft YaHei", 12);
    m_textEdit->setFont(font);

    // 发送按钮
    m_sendButton = new QPushButton("发送");
    m_sendButton->setObjectName("sendButton");
    m_sendButton->setFixedSize(60, 30);
    m_sendButton->setEnabled(false); // 初始禁用

    // 发送按钮布局
    QHBoxLayout *sendButtonLayout = new QHBoxLayout;
    sendButtonLayout->setContentsMargins(0,0,0,0);
    sendButtonLayout->addStretch(); // 添加弹性空间
    sendButtonLayout->addWidget(m_sendButton);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(m_textEdit,1);
    mainLayout->addStretch();  // 这会让按钮保持在底部
    mainLayout->addLayout(sendButtonLayout); // 使用布局而不是直接添加按钮
}

void InputArea::setupConnections()
{
    connect(m_sendButton, &QPushButton::clicked, this, &InputArea::do_send_clicked);
    connect(m_imageButton, &QToolButton::clicked, this, &InputArea::do_image_clicked);
    connect(m_videoButton, &QToolButton::clicked, this, &InputArea::do_video_clicked);
    connect(m_audioButton, &QToolButton::clicked, this, &InputArea::do_audio_clicked);
    connect(m_fileButton, &QToolButton::clicked, this, &InputArea::do_file_clicked);
    connect(m_textEdit, &QTextEdit::textChanged, this, &InputArea::do_text_changed);
    connect(m_captureButton, &QToolButton::clicked, this, &InputArea::do_capture_clicked);
    connect(m_fileButton, &QToolButton::clicked, this, &InputArea::do_file_clicked);
    connect(this,&InputArea::on_message_sent,this,&InputArea::do_message_sent);
}

void InputArea::setupEmojiMenu()
{
    m_emojiMenu = new QMenu(this);
    m_emojiMenu->setObjectName("emojiMenu");
    // 常用表情分类
    QStringList emojiCategories = {
        // 第一类：开心表情
        "😀", "😃", "😄", "😁", "😆", "😅", "😂", "🤣", "😊", "😇",

        // 第二类：爱心和亲吻表情
        "🙂", "🙃", "😉", "😌", "😍", "🥰", "😘", "😗", "😙", "😚",

        // 第三类：搞笑和聪明表情
        "😋", "😛", "😝", "😜", "🤪", "🤨", "🧐", "🤓", "😎", "🤩",

        // 第四类：庆祝和消极表情
        "🥳", "😏", "😒", "😞", "😔", "😟", "😕", "🙁", "☹️", "😣",

        // 第五类：悲伤和生气表情
        "😖", "😫", "😩", "🥺", "😢", "😭", "😤", "😠", "😡",

        // 第六类：爱心颜色
        "❤️", "🧡", "💛", "💚", "💙", "💜", "🖤", "🤍", "🤎",

        // 第七类：手势
        "👍", "👎", "👌", "🤏", "✌️", "🤞", "🤟", "🤘", "👈", "👉",

        // 第八类：动物
        "🐶", "🐱", "🐭", "🐹", "🐰", "🦊", "🐻", "🐼", "🐨", "🐯"
    };

    // 每行显示10个表情
    int emojisPerRow = 10;

    for (int i = 0; i < emojiCategories.size(); i += emojisPerRow) {
        QWidget *emojiWidget = new QWidget;
        QHBoxLayout *layout = new QHBoxLayout(emojiWidget);
        layout->setAlignment(Qt::AlignHCenter);
        layout->setSpacing(2);
        layout->setContentsMargins(2, 2, 2, 2);

        // 添加当前行的表情
        for (int j = i; j < qMin(i + emojisPerRow, emojiCategories.size()); j++) {
            QString emoji = emojiCategories[j];

            QPushButton *emojiBtn = new QPushButton(emoji);
            emojiBtn->setFixedSize(30, 30);
            emojiBtn->setFont(QFont("Segoe UI Emoji", 12));
            emojiBtn->setStyleSheet("QPushButton{"
                                    "background:#eceeb4;"
                                    "border:1px;"
                                    "}"
                                    "QPushButton:hover{"
                                    "background:#caeeb4;"
                                    "}");

            connect(emojiBtn, &QPushButton::clicked, [this, emoji]() {
                insertEmoji(emoji);
            });
            layout->addWidget(emojiBtn);
        }

        QWidgetAction *action = new QWidgetAction(m_emojiMenu);
        action->setDefaultWidget(emojiWidget);
        m_emojiMenu->addAction(action);
    }

    m_emojiButton->setMenu(m_emojiMenu);
}

void InputArea::showAudioDialog()
{
    QDialog*recordDialog = new QDialog(this);
    recordDialog->setWindowTitle("录音");
    // recordDialog->setAttribute(Qt::WA_TranslucentBackground);
    recordDialog->setFixedSize({400,300});
    recordDialog->setStyleSheet(R"(
        QDialog {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #2c3e50, stop:1 #34495e);
            color: white;
            border-radius:12px;
        }
        QPushButton {
            background: #3498db;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            font-size: 14px;
            min-width: 50px;
        }
        QPushButton:hover {
            background: #2980b9;
        }
        QPushButton:pressed {
            background: #21618c;
        }
        QPushButton[state="recording"] {
            background: #e74c3c;
        }
        QPushButton[state="recording"]:hover {
            background: #c0392b;
        }
        QPushButton[state="stopped"] {
            background: #2ecc71;
        }
        QPushButton[state="stopped"]:hover {
            background: #27ae60;
        }
        QLabel {
            color: white;
            font-size: 14px;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(recordDialog);

    // 波形图
    QLabel *waveformLabel = new QLabel;
    waveformLabel->setFixedHeight(120);
    waveformLabel->setStyleSheet("background: #1a252f; border-radius: 10px;");
    waveformLabel->setAlignment(Qt::AlignCenter);

    // 时间
    QLabel *timeLabel = new QLabel("00:00");
    timeLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #ecf0f1;");
    timeLabel->setAlignment(Qt::AlignCenter);

    // 状态显示
    QLabel *statusLabel = new QLabel("准备录音");
    statusLabel->setAlignment(Qt::AlignCenter);

    // 控制按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;

    QPushButton *recordButton = new QPushButton("开始录音");
    recordButton->setProperty("state", "stopped");

    QPushButton *playButton = new QPushButton("播放");
    playButton->setEnabled(false);

    QPushButton *sendButton = new QPushButton("发送");
    sendButton->setEnabled(false);

    QPushButton *cancelButton = new QPushButton("取消");

    buttonLayout->addWidget(recordButton);
    buttonLayout->addWidget(playButton);
    buttonLayout->addWidget(sendButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addWidget(waveformLabel);
    mainLayout->addWidget(timeLabel);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    // 录音相关变量
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    QAudioSource *audioInput = nullptr;
    QBuffer *audioBuffer = new QBuffer;
    QTimer *recordTimer = new QTimer;
    int recordSeconds = 0;

    QVector<qreal> audioLevels;
    QTimer *waveformTimer = new QTimer;

    // 开始/停止录音
    connect(recordButton, &QPushButton::clicked, [=,this, &audioInput, &audioBuffer, &recordSeconds]() mutable {
        if (recordButton->property("state").toString() == "stopped") {
            // 开始录音
            QAudioDevice device = QMediaDevices::defaultAudioInput();
            if (device.isNull()) {
                QMessageBox::warning(this, "错误", "未找到录音设备");
                return;
            }

            if (!device.isFormatSupported(format)) {
                QMessageBox::warning(this, "错误", "音频格式不支持");
                return;
            }

            audioInput = new QAudioSource(format, recordDialog);
            audioBuffer->open(QIODevice::WriteOnly);

            audioInput->start(audioBuffer);

            recordButton->setProperty("state", "recording");
            recordButton->setText("停止录音");
            statusLabel->setText("录音中...");
            playButton->setEnabled(false);
            sendButton->setEnabled(false);

            // 开始计时
            recordSeconds = 0;
            recordTimer->start(1000);
            waveformTimer->start(50); // 更新波形

        } else {
            // 停止录音
            if (audioInput) {
                audioInput->stop();
                audioInput->deleteLater();
                audioInput = nullptr;
            }
            audioBuffer->close();

            recordButton->setProperty("state", "stopped");
            recordButton->setText("开始录音");
            statusLabel->setText("录音完成");
            playButton->setEnabled(true);
            sendButton->setEnabled(true);

            recordTimer->stop();
            waveformTimer->stop();
        }

        // 更新样式
        recordDialog->style()->unpolish(recordButton);
        recordDialog->style()->polish(recordButton);
        recordButton->update();
    });
    // 更新时间显示
    connect(recordTimer, &QTimer::timeout, [=, &recordSeconds]() {
        recordSeconds++;
        int minutes = recordSeconds / 60;
        int seconds = recordSeconds % 60;
        timeLabel->setText(QString("%1:%2")
                               .arg(minutes, 2, 10, QLatin1Char('0'))
                               .arg(seconds, 2, 10, QLatin1Char('0')));
    });

    // 更新波形显示
    connect(waveformTimer, &QTimer::timeout, [=,this]() mutable{
        updateWaveform(waveformLabel, audioLevels);
    });

    // 播放录音
    connect(playButton, &QPushButton::clicked, [=,this]() {
        if (audioBuffer && audioBuffer->size() > 0) {
            playAudio(audioBuffer->data());
        }
    });

    connect(sendButton, &QPushButton::clicked, [=,this]() {
        if (audioBuffer && audioBuffer->size() > 0) {
            sendAudioMessage(audioBuffer->data(), recordSeconds);
            recordDialog->accept();
        }
    });

    connect(cancelButton, &QPushButton::clicked, recordDialog, &QDialog::reject);

    recordDialog->exec();

    // 清理资源
    if (audioInput) {
        audioInput->stop();
        audioInput->deleteLater();
    }
    audioBuffer->deleteLater();
    recordTimer->deleteLater();
    waveformTimer->deleteLater();
}


void InputArea::updateWaveform(QLabel *waveformLabel, QVector<qreal> &audioLevels)
{
    // 模拟音频电平数据（实际应该从音频输入获取）
    static qreal level = 0;
    level =-0.5 + (qSin(QDateTime::currentMSecsSinceEpoch() / 100.0) + 1) / 2;

    // 保持最近50个电平值
    audioLevels.append(level);
    if (audioLevels.size() > 50) {
        audioLevels.removeFirst();
    }

    // 创建波形图
    QPixmap pixmap(waveformLabel->width(), waveformLabel->height());
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制网格
    painter.setPen(QPen(QColor(255, 255, 255, 50), 1));
    for (int i = 0; i < waveformLabel->width(); i += 20) {
        painter.drawLine(i, 0, i, waveformLabel->height());
    }
    for (int i = 0; i < waveformLabel->height(); i += 20) {
        painter.drawLine(0, i, waveformLabel->width(), i);
    }

    // 绘制中心线
    painter.setPen(QPen(QColor(255, 255, 255, 100), 2));
    int centerY = waveformLabel->height() / 2;
    painter.drawLine(0, centerY, waveformLabel->width(), centerY);

    // 绘制波形
    if (!audioLevels.isEmpty()) {
        painter.setPen(QPen(QColor(0, 255, 255), 3));

        int step = waveformLabel->width() / audioLevels.size();
        for (int i = 0; i < audioLevels.size() - 1; ++i) {
            int x1 = i * step;
            int x2 = (i + 1) * step;
            int y1 = centerY - audioLevels[i] * centerY;
            int y2 = centerY - audioLevels[i + 1] * centerY;

            painter.drawLine(x1, y1, x2, y2);
        }

        // 绘制实时电平点
        painter.setBrush(QBrush(QColor(255, 100, 100)));
        painter.setPen(Qt::NoPen);
        int lastX = (audioLevels.size() - 1) * step;
        int lastY = centerY - audioLevels.last() * centerY;
        painter.drawEllipse(lastX - 3, lastY - 3, 6, 6);
    }

    waveformLabel->setPixmap(pixmap);
}

void InputArea::playAudio(const QByteArray &audioData)
{
    // 实现音频播放
    QBuffer *playBuffer = new QBuffer;
    playBuffer->setData(audioData);
    playBuffer->open(QIODevice::ReadOnly);

    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    QAudioSink *audioOutput = new QAudioSink(format, this);
    audioOutput->start(playBuffer);

    // 播放完成后清理
    connect(audioOutput, &QAudioSink::stateChanged, [=](QAudio::State state) {
        if (state == QAudio::IdleState) {
            audioOutput->stop();
            audioOutput->deleteLater();
            playBuffer->deleteLater();
        }
    });
}

void InputArea::sendAudioMessage(const QByteArray &audioData, int duration)
{
    // 保存音频文件
    QString tempPath = QDir::tempPath() + "/audio_" +
                       QString::number(QDateTime::currentMSecsSinceEpoch()) + ".wav";

    // 这里需要添加WAV文件头并保存数据
    // 简化处理：直接保存原始数据（实际应该格式化为WAV）
    QByteArray wavData = createWavFile(audioData, duration);

    QFile file(tempPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(wavData);
        file.close();
    }

    // 创建音频消息
    MessageItem item;
    item.content.type = MessageType::AudioMessage;

    MessageContent content;
    content.type = MessageType::AudioMessage;
    content.data = tempPath;
    content.mimeType = "audio/pcm";

    item.content = content;
    item.env = MessageEnv::Private;
    item.from_id = UserManager::GetInstance()->GetUid();
    item.to_id = UserManager::GetInstance()->GetPeerUid();

    emit on_message_sent(item);

}

QByteArray InputArea::createWavFile(const QByteArray &pcmData, int duration) const
{
    int sampleRate = 44100;
    int numChannels = 1;
    int bitsPerSample = 16;
    int byteRate = sampleRate * numChannels * bitsPerSample / 8;
    int blockAlign = numChannels * bitsPerSample / 8;
    int dataSize = pcmData.size();
    int fileSize = 36 + dataSize;

    QByteArray wavData;
    QDataStream stream(&wavData, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    // RIFF header
    stream.writeRawData("RIFF", 4);
    stream << quint32(fileSize);
    stream.writeRawData("WAVE", 4);

    // fmt chunk
    stream.writeRawData("fmt ", 4);
    stream << quint32(16);        // Chunk size
    stream << quint16(1);         // Audio format (1 = PCM)
    stream << quint16(numChannels); // Number of channels
    stream << quint32(sampleRate);   // Sample rate
    stream << quint32(byteRate);     // Byte rate
    stream << quint16(blockAlign);   // Block align
    stream << quint16(bitsPerSample); // Bits per sample

    // data chunk
    stream.writeRawData("data", 4);
    stream << quint32(dataSize);     // Data size

    // 添加 PCM 数据
    wavData.append(pcmData);

    return wavData;
}

void InputArea::insertImage(const QString &imagePath)
{
    QPixmap roundedPixmap = SourceManager::GetInstance()->getPixmap(imagePath+"_rounded");
    if (roundedPixmap.isNull()) {
        QMessageBox::warning(this,"异常","图片不存在或者文件损坏",QMessageBox::Ok);
        return;
    }

    QTextCursor cursor = m_textEdit->textCursor();

    // 创建图片格式
    QTextImageFormat imageFormat;
    imageFormat.setName(imagePath);
    imageFormat.setWidth(100);
    imageFormat.setHeight(100);

    // 插入图片
    cursor.insertImage(imageFormat);
    cursor.insertText(" ");

    m_textEdit->setFocus();

}


void InputArea::insertImageFromClipboard(const QImage &image)
{
    // 保存为临时文件
    QString tempPath = QDir::tempPath() + "/tmp_from_quick_chat_clipboard_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".png";
    if (!image.save(tempPath, "PNG")) {
        return;
    }
    // 复用你已有的插入逻辑
    insertImage(tempPath);
}

std::optional<QList<MessageItem>> InputArea::parseMessageContent()
{
    QTextDocument* doc = m_textEdit->document();
    QTextBlock currentBlock = doc->begin();
    QList<MessageItem> item_list;

    int peerUid = UserManager::GetInstance()->GetPeerUid();
    if (peerUid < 0){
        return std::nullopt;
    }

    // 用于累积文本内容
    QString accumulatedText;

    while (currentBlock.isValid()) {
        QTextBlock::Iterator it;
        for (it = currentBlock.begin(); !(it.atEnd()); ++it) {
            QTextFragment fragment = it.fragment();
            if (fragment.isValid()) {
                QTextCharFormat format = fragment.charFormat();
                if (format.isImageFormat()) {
                    // 如果遇到图片，先处理累积的文本（如果有的话）
                    if (!accumulatedText.trimmed().isEmpty()) {
                        MessageItem textItem;
                        textItem.timestamp = QDateTime::currentDateTime();
                        textItem.env = UserManager::GetInstance()->GetEnv();
                        textItem.from_id = UserManager::GetInstance()->GetUid();
                        textItem.to_id = peerUid;
                        textItem.content.mimeType = "text/plain";
                        textItem.content.type = MessageType::TextMessage;
                        textItem.content.data = accumulatedText;
                        textItem.status = 0;
                        item_list.append(textItem);
                        accumulatedText.clear(); // 清空累积的文本
                    }

                    // 处理图片
                    QTextImageFormat imageFormat = format.toImageFormat();
                    QString imagePath = imageFormat.name();
                    QMimeDatabase db;
                    QMimeType mime = db.mimeTypeForFile(imagePath);

                    MessageItem imageItem;
                    imageItem.env = UserManager::GetInstance()->GetEnv();
                    imageItem.from_id = UserManager::GetInstance()->GetUid();
                    imageItem.to_id = peerUid;
                    imageItem.status = 0;
                    imageItem.content.mimeType = mime.name();
                    imageItem.content.type = MessageType::ImageMessage;
                    imageItem.content.data = imagePath;

                    item_list.append(imageItem);
                } else {
                    // 累积文本内容
                    QString text = fragment.text();
                    accumulatedText += text;
                }
            }
        }
        // 块结束时添加换行符（如果需要保持段落结构）
        if (currentBlock.next().isValid()) {
            accumulatedText += "\n";
        }
        currentBlock = currentBlock.next();
    }

    // 处理最后累积的文本（如果有的话）
    if (!accumulatedText.trimmed().isEmpty()) {
        MessageItem textItem;
        textItem.env = UserManager::GetInstance()->GetEnv();
        textItem.from_id = UserManager::GetInstance()->GetUid();
        textItem.to_id = peerUid;
        textItem.status = 0;
        textItem.content.mimeType = "text/plain";
        textItem.content.type = MessageType::TextMessage;
        textItem.content.data = accumulatedText;
        item_list.append(textItem);
    }

    return item_list.size() == 0 ? std::nullopt : std::make_optional(item_list);
}

void InputArea::do_send_clicked()
{
    std::optional<QList<MessageItem>> list = parseMessageContent();
    if (list.has_value()) {
        for(const auto&item:list.value()){
            if (item.content.type == MessageType::TextMessage){
                if (item.content.data.toString().size() > 2048){
                    QMessageBox msgBox;
                    msgBox.setWindowTitle("Too Long Text！");
                    msgBox.setText("Too Long Text!");
                    msgBox.setIcon(QMessageBox::Warning);
                    msgBox.setStandardButtons(QMessageBox::Ok);

                    // macOS 风格样式表
                    msgBox.setStyleSheet(R"(
                        QMessageBox {
                            background-color: #f5f5f7;
                            border: 1px solid #d0d0d0;
                            border-radius: 10px;
                            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
                        }
                        QMessageBox QLabel {
                            color: #1d1d1f;
                            font-size: 14px;
                            font-weight: 400;
                            padding: 15px;
                        }
                        QMessageBox QLabel#qt_msgbox_label {
                            min-width: 300px;
                        }
                        QMessageBox QPushButton {
                            background-color: #007aff;
                            color: white;
                            border: none;
                            border-radius: 6px;
                            padding: 8px 24px;
                            font-size: 13px;
                            font-weight: 500;
                            min-width: 80px;
                            margin: 5px;
                        }
                        QMessageBox QPushButton:hover {
                            background-color: #0056d6;
                        }
                        QMessageBox QPushButton:pressed {
                            background-color: #0040a8;
                        }
                        QMessageBox QPushButton:focus {
                            outline: 2px solid #007aff;
                            outline-offset: 2px;
                        }
                    )");

                    msgBox.exec();
                }else{
                    emit on_message_sent(item);
                    clear();
                }
            }else{
                emit on_message_sent(item);
                clear();
            }
        }
    }
}

void InputArea::do_emoji_clicked()
{
    // 菜单已经在按钮点击时自动显示
}

void InputArea::do_image_clicked()
{
    QString imagePath = QFileDialog::getOpenFileName(
        this,
        "选择图片",
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        "图片文件 (*.png *.jpg *.jpeg *.gif *.bmp)"
    );

    if (!imagePath.isEmpty()) {
        // 创建消息项并发送
        MessageItem item;
        item.content.type = MessageType::ImageMessage;
        item.to_id = UserManager::GetInstance()->GetPeerUid();
        item.content.mimeType = "image/*";
        item.content.data = imagePath;

        emit on_message_sent(item);
    }
}

void InputArea::do_audio_clicked()
{
    showAudioDialog();
}

void InputArea::do_video_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("选择视频文件"),
                                                    "",
                                                    tr("视频文件 (*.mp4 *.avi *.mov *.mkv *.wmv)"));

    if (!filePath.isEmpty()) {
        // 创建消息项并发送
        MessageItem item;
        item.content.type = MessageType::VideoMessage;
        item.to_id = UserManager::GetInstance()->GetPeerUid();

        MessageContent content;
        content.type = MessageType::VideoMessage;
        content.data = filePath;
        content.mimeType = "video/*";

        item.content = content;

        emit on_message_sent(item);
    }
}

void InputArea::do_text_changed()
{
    // 检查是否有内容（去除空白字符）
    bool hasText = !m_textEdit->toPlainText().trimmed().isEmpty();
    m_sendButton->setEnabled(hasText);
}

void InputArea::do_file_clicked()
{
    // 定义排除的文件类型
    QStringList excludeFilters;
    excludeFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.gif" << "*.bmp" << "*.webp"  // 图片
                   << "*.mp4" << "*.avi" << "*.mov" << "*.mkv" << "*.wmv" << "*.flv" << "*.webm"; // 视频

    // 构建文件过滤器（排除图片和视频）
    QString fileFilter = tr("所有文件 (*)");

    // 或者可以设置一些常见的文档类型
    QString documentFilter = tr("文档文件 (*.pdf *.doc *.docx *.xls *.xlsx *.ppt *.pptx *.txt *.zip *.rar *.7z)");

    QString selectedFilter;
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        tr("选择文件"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        documentFilter + ";;" + fileFilter,
        &selectedFilter
        );

    if (filePaths.isEmpty()) {
        return;
    }

    // 过滤掉图片和视频文件
    QStringList filteredFiles;
    for (const QString &filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        QString suffix = fileInfo.suffix().toLower();

        // 检查是否是图片或视频文件
        bool isImage = (suffix == "png" || suffix == "jpg" || suffix == "jpeg" ||
                        suffix == "gif" || suffix == "bmp" || suffix == "webp");
        bool isVideo = (suffix == "mp4" || suffix == "avi" || suffix == "mov" ||
                        suffix == "mkv" || suffix == "wmv" || suffix == "flv" || suffix == "webm");

        if (!isImage && !isVideo) {
            filteredFiles.append(filePath);
        } else {
            qDebug() << "跳过文件（图片或视频类型）:" << filePath;
        }
    }

    if (filteredFiles.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("选择的文件都是图片或视频类型，请使用对应的图片或视频按钮发送。"));
        return;
    }

    // 发送文件消息
    for (const QString &filePath : filteredFiles) {
        if (!filePath.isEmpty()) {
            // 创建消息项并发送
            MessageItem item;
            item.content.type = MessageType::OtherFileMessage;
            item.to_id = UserManager::GetInstance()->GetPeerUid();

            item.content.type = MessageType::OtherFileMessage;
            item.content.data = filePath;
            item.content.mimeType = getMimeTypeForFile(filePath);
            item.content.fid = QUuid::createUuid().toString();

            emit on_message_sent(item);
        }
    }
}

QString InputArea::getMimeTypeForFile(const QString &filePath)
{
    QMimeDatabase mimeDatabase;
    QMimeType mimeType = mimeDatabase.mimeTypeForFile(filePath);
    return mimeType.name();
}



void InputArea::do_capture_clicked()
{
    // TODO:截图
}

void InputArea::do_message_sent(const MessageItem &item)
{
    if (!TcpManager::GetInstance()->isConnected()){
        QMessageBox msgBox;
        msgBox.setWindowTitle("Network Issue");
        msgBox.setText("No Connection to Server");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);

               // macOS 风格样式表
        msgBox.setStyleSheet(R"(
            QMessageBox {
                background-color: #f5f5f7;
                border: 1px solid #d0d0d0;
                border-radius: 10px;
                font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            }
            QMessageBox QLabel {
                color: #1d1d1f;
                font-size: 14px;
                font-weight: 400;
                padding: 15px;
            }
            QMessageBox QLabel#qt_msgbox_label {
                min-width: 300px;
            }
            QMessageBox QPushButton {
                background-color: #007aff;
                color: white;
                border: none;
                border-radius: 6px;
                padding: 8px 24px;
                font-size: 13px;
                font-weight: 500;
                min-width: 80px;
                margin: 5px;
            }
            QMessageBox QPushButton:hover {
                background-color: #0056d6;
            }
            QMessageBox QPushButton:pressed {
                background-color: #0040a8;
            }
            QMessageBox QPushButton:focus {
                outline: 2px solid #007aff;
                outline-offset: 2px;
            }
        )");

        msgBox.exec();
        return;
    }
    m_model->addMessage(item);
    auto pb = toPb(item);
    std::string pb_str = pb.SerializeAsString();
    qDebug() << pb_str;
    QByteArray ba(pb_str.data(),pb_str.size());

    TcpManager::GetInstance()->do_send_data(RequestType::ID_TEXT_CHAT_MSG_REQ,ba);
    DataBase::GetInstance().storeMessage(item);
}

QString InputArea::getText() const
{
    return m_textEdit->toPlainText();
}

void InputArea::clear()
{
    m_textEdit->clear();
    m_sendButton->setEnabled(false);
}

void InputArea::insertEmoji(const QString &emoji)
{
    m_textEdit->textCursor().insertText(emoji);
    m_textEdit->setFocus();
}

void InputArea::insertText(const QString&text){
    m_textEdit->textCursor().insertText(text);
    m_textEdit->setFocus();
}


void InputArea::setModel(MessageModel *model)
{
    this->m_model = model;
}

QPixmap InputArea::createRoundedPixmap(const QString &imagePath, int size, int radius)
{
    // 加载原始图片
    QPixmap original(imagePath);
    if (original.isNull()) {
        return QPixmap();
    }

    // 缩放图片
    // QPixmap scaled = original.scaled(size,size,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
    // QPixmap scaled = original.scaled(200, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QPixmap scaled = SourceManager::GetInstance()->getPixmap(imagePath+"_rounded");

    // // 创建带圆角的遮罩
    QPixmap rounded(scaled.size());
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 创建圆角路径
    QPainterPath path;
    path.addRoundedRect(0, 0, scaled.width(), scaled.height(), radius, radius);

    painter.setClipPath(path);
    painter.drawPixmap(0, 0, scaled);

    return rounded;
}

// 事件过滤，支持 Ctrl+Enter 发送
bool InputArea::eventFilter(QObject *obj, QEvent *event)
{
    // 只处理 QTextEdit 的键盘事件
    if (obj == m_textEdit && event->type() == QEvent::KeyPress) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        // 粘贴
        if (keyEvent->matches(QKeySequence::Paste)) {
            const QClipboard *clipboard = QApplication::clipboard();
            const QMimeData *mimeData = clipboard->mimeData();

            // ① 优先粘贴图片
            if (mimeData->hasImage()) {
                QImage image = qvariant_cast<QImage>(mimeData->imageData());
                if (!image.isNull()) {
                    insertImageFromClipboard(image);
                    return true; // 已处理，不再走默认粘贴
                }
            }

            // ② 如果不是图片，走默认粘贴（文本）
            return false;
        }

        // 处理回车键
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() & Qt::ShiftModifier) {
                // Shift+Enter：允许默认行为（插入换行）
                return false;  // 继续传播，让 QTextEdit 插入换行
            } else {
                do_send_clicked();
                return true;   // 事件已处理，阻止 QTextEdit 的默认行为
            }
        }
    }

    // 其他事件交给基类处理
    return QWidget::eventFilter(obj, event);
}

