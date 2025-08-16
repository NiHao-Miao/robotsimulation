#ifndef ATTACHMENTDIALOG_H
#define ATTACHMENTDIALOG_H

#include <QDialog>

namespace Ui {
class AttachmentDialog;
}

class AttachmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AttachmentDialog(QWidget *parent = nullptr);
    ~AttachmentDialog();

    // --- 新增：公共接口 ---
    /**
         * @brief 设置下拉列表中可供选择的目标名称。
         */
    void setAttachmentTargets(const QStringList& targetNames);

    /**
         * @brief 获取用户最终选择的目标名称。
         */
    QString getSelectedTarget() const;

    // --- 用于操作新下拉菜单的公共接口 ---
    /**
         * @brief 设置可供追踪的连杆ID列表。
         */
    void setTrackableLinks(const QStringList& linkIds);

    /**
         * @brief 获取用户选择的要追踪的连杆ID。
         */
    QString getSelectedLink() const;

private:
    Ui::AttachmentDialog *ui;
};

#endif // ATTACHMENTDIALOG_H
