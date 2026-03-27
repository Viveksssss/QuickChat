#ifndef MESSAGEDELEGATE_H
#define MESSAGEDELEGATE_H

#include <QStyledItemDelegate>
#include <QVideoFrame>
#include "messagetypes.h"

class MessageArea;
class MessageDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit MessageDelegate(QObject *parent = nullptr);
public:
    // QAbstractItemDelegate interface
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setMessageArea(MessageArea*area);
protected:

    int paintTextMessage(QPainter *painter, const QRect &rect,const MessageContent &content, const QStyleOptionViewItem &option)const;
    int paintImageMessage(QPainter*painter,const QRect &rect,const MessageContent,const QStyleOptionViewItem&option)const;
    void paintSelectionIndicator(QPainter *painter, const QRect &rect, bool selected)const;
private:
    void paintAvatar(QPainter *painter, const QRect &rect, const QPixmap &avatar) const;
    void paintUserName(QPainter*painter,const QRect&rect,const QString&name,bool isMe)const;
    void paintButtleBackground(QPainter *painter, const QRect &rect,const QColor &color)const;
    QSize calculateMessageSize(const MessageContent&contents,const QStyleOptionViewItem &option)const;
    bool containsManyEmojis(const QString& text) const;
    void showContextMenu(const QPoint &globalPos, const QModelIndex &index);
    int textAvailWidth(const QStyleOptionViewItem &option,bool showAvatar) const;


    // 渲染气泡框
    void paintBubbleMessage(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index, const MessageContent&content,bool isMe, const QColor &bubbleColor, const QString &timeText) const;

    // 渲染图片
    void paintRoundedImage(QPainter *painter, const QRect &rect, const QString &imagePath) const;

    // 渲染文件消息
    void paintFileMessageContent(QPainter *painter, const QRect &rect, const MessageContent &content) const;
    void paintFileIcon(QPainter *painter, const QRect &rect, const QString &fileSuffix) const;

    // 渲染视频消息
    void paintVideoMessageContent(QPainter *painter, const QRect &rect, const MessageContent &content) const;
    QPixmap getVideoThumbnail(const QString &videoPath, const QSize &size) const;
    QPixmap videoFrameToPixmap(const QVideoFrame &frame) const;
    void paintPlayButton(QPainter *painter, const QRect &rect) const;
    void paintVideoFileName(QPainter *painter, const QRect &rect, const QString &videoPath) const;
    void paintVideoIcon(QPainter *painter, const QRect &rect) const;

    // 渲染音频信息
    void paintAudioMessageContent(QPainter *painter, const QRect &rect, const MessageContent &content) const;
    void paintAudioPlayButton(QPainter *painter, const QRect &rect) const;
    void paintAudioInfo(QPainter *painter, const QRect &rect, const QString &fileName, const QString &duration) const;
    void paintAudioWaveform(QPainter *painter, const QRect &rect) const;
    QString getAudioDuration(const QString &audioPath) const;
    QString formatTime(qint64 milliseconds) const;

    MessageType parseType(const MessageContent &content) const;
    void paintPureOtherFileMessage(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const MessageContent &contents,bool isMe, const QString &timeText) const;
    void paintPureImageMessage(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const MessageContent&contents,bool isMe, const QString &timeText) const;
    void paintPureVideoMessage(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const MessageContent&contents,bool isMe, const QString &timeText) const;
    void paintPureAudioMessage(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const MessageContent&contents,bool isMe, const QString &timeText) const;


    QString cleanFilePath(const QString &filePath) const;
    void openFile(const QString &filePath) const;
    void openImage(const QString&filePath) const;
    void openAudio(const QString&filePath) const;
    void openVideo(const QString &videoPath) const;
protected:
    const int AVATAR_SIZE = 40;
    const int AVATAR_MARGIN = 8;
    const int BUBBLE_PADDING = 8;
    const int BUBBLE_RADIUS = 12;
    const int MESSAGE_SPACING = 4;
    const int SELECTION_INDICATOR_SIZE = 20;
    const int USER_NAME_HEIGHT = 16;

    // QAbstractItemDelegate interface
public:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
private:
    QString cachedMessageText; // 缓存要复制的消息文本
    QMenu *menu;
    MessageArea*area;
    QAction *copyAction ;
    QAction *selectAction;
    QAction *deleteAction;
    QAction *resendAction;

};

// 支持滚轮查看的label
class ImageViewer : public QWidget
{
    Q_OBJECT
public:
    explicit ImageViewer(const QPixmap &pix, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
    void wheelEvent(QWheelEvent *ev) override;

private:
    QPixmap m_pix;
    qreal m_scale = 1.0;   // 当前缩放
    qreal m_scaleStep = 0.1;
    qreal m_minScale = 0.2;
    qreal m_maxScale = 5.0;
};

#endif // MESSAGEDELEGATE_H
