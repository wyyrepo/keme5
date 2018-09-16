#ifndef LISTADOCS_H
#define LISTADOCS_H

#include <QtGui>
#include "ui_listadocs.h"
#include <QSqlTableModel>

class listadocs : public QDialog {
    Q_OBJECT
        public:
                listadocs();
                ~listadocs();

        private:
           Ui::listadocs ui;
           QSqlTableModel *modeloreg;
       private slots :
               void compruebacodigo();
               void codigocambiado();
               void descripcioncambiada();
               void botonguardarpulsado();
               void botoncancelarpulsado();
               void botoneliminarpulsado();
               void tablapulsada();
               void eddoc();
};

#endif // LISTADOCS_H
