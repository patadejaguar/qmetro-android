#ifndef DETAILS_H
#define DETAILS_H

#include <QDialog>

namespace Ui {
    class Details;
}

class Details : public QDialog
{
    Q_OBJECT

public:
    explicit Details(QWidget *parent = 0);
    ~Details();

    Ui::Details *ui;

signals:
    void resizeWindow();
    void showed();

private:
    void closeEvent(QCloseEvent *e);

protected:
    void keyPressEvent(QKeyEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);
    void showEvent(QShowEvent *e);

};

#endif // DETAILS_H
