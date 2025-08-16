#ifndef MULTIRECORDDIALOG_H
#define MULTIRECORDDIALOG_H

#include <QDialog>

namespace Ui {
class MultiRecordDialog;
}

class MultiRecordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MultiRecordDialog(QWidget *parent = nullptr);
    ~MultiRecordDialog();

    void setLinks(const QStringList& linkIds);
    void setReferenceFrames(const QStringList& frameNames);
    QStringList getSelectedLinks() const;
    QString getSelectedReferenceFrame() const;

private:
    Ui::MultiRecordDialog *ui;
};

#endif // MULTIRECORDDIALOG_H
