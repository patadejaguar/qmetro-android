#include <QKeyEvent>
#include "details.h"
#include "ui_details.h"

Details::Details(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Details)
{
 setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
 installEventFilter(this);
 ui->setupUi(this);
 setAttribute(Qt::WA_DeleteOnClose);
}

Details::~Details()
{
 delete ui;
}

void Details::closeEvent(QCloseEvent *e)
{Q_UNUSED(e);
 removeEventFilter(this);
 //delete this;
 //deleteLater();
}

bool Details::eventFilter(QObject *obj, QEvent *e)
{Q_UNUSED(obj);
 QDialog::event(e);
 if(e->type()==QEvent::Resize) emit resizeWindow();
 return 1;
}

void Details::showEvent(QShowEvent *e)
{Q_UNUSED(e);
 emit showed();
}

void Details::keyPressEvent(QKeyEvent *e)
{
 if(e->key()==Qt::Key_Escape) close();
}
