#ifndef INPUTAREA_H
#define INPUTAREA_H

#include <QTextEdit>
#include <QToolButton>
#include <QWidget>
#include "../MessageArea/messagetypes.h"

class QLabel;
class QPushButton;

class MessageModel;
class InputArea : public QWidget
{
    Q_OBJECT

public:
    explicit InputArea(QWidget *parent = nullptr);
    ~InputArea();


    QString getText() const;
    void clear();
    void insertText(const QString&text);
    void insertEmoji(const QString &emoji);
    void setModel(MessageModel*model);

signals:
    void on_message_sent(const MessageItem &item);

private slots:
    void do_send_clicked();
    void do_emoji_clicked();
    void do_image_clicked();
    void do_audio_clicked();
    void do_video_clicked();
    void do_text_changed();
    void do_file_clicked();
    void do_capture_clicked();
    void do_message_sent(const MessageItem&item);

private:
    void setupUI();
    void setupConnections();
    void setupEmojiMenu();
    void insertRichText(const QString &html);
    void insertImageFromClipboard(const QImage &image);
    void insertImage(const QString &imagePath);
    QString getMimeTypeForFile(const QString &filePath);
    // 弹出窗口用于录音
    void showAudioDialog();
    void updateWaveform(QLabel *waveformLabel, QVector<qreal> &audioLevels);
    void playAudio(const QByteArray &audioData);
    void sendAudioMessage(const QByteArray &audioData, int duration);
    QByteArray createWavFile(const QByteArray &pcmData, int duration) const;

    std::optional<QList<MessageItem>> parseMessageContent();

    QPixmap createRoundedPixmap(const QString &imagePath, int size, int radius);

    QTextEdit *m_textEdit;
    QToolButton *m_emojiButton;
    QToolButton *m_videoButton;
    QToolButton *m_audioButton;
    QToolButton *m_imageButton;
    QToolButton *m_fileButton;
    QToolButton *m_captureButton;
    QPushButton *m_sendButton;
    QMenu *m_emojiMenu;
    MessageModel*m_model;

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
};

#endif // INPUTAREA_H
