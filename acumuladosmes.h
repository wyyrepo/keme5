/* ----------------------------------------------------------------------------------
    KEME-Contabilidad; aplicación para llevar contabilidades

    Copyright (C)  José Manuel Díez Botella

    Este programa es software libre: usted puede redistribuirlo y/o modificarlo 
    bajo los términos de la Licencia Pública General GNU publicada 
    por la Fundación para el Software Libre, ya sea la versión 3 
    de la Licencia, o (a su elección) cualquier versión posterior.

    Este programa se distribuye con la esperanza de que sea útil, pero 
    SIN GARANTÍA ALGUNA; ni siquiera la garantía implícita 
    MERCANTIL o de APTITUD PARA UN PROPÓSITO DETERMINADO. 
    Consulte los detalles de la Licencia Pública General GNU para obtener 
    una información más detallada. 

    Debería haber recibido una copia de la Licencia Pública General GNU 
    junto a este programa. 
    En caso contrario, consulte <http://www.gnu.org/licenses/>.
  ----------------------------------------------------------------------------------*/

#ifndef ACUMULADOSMES_H
#define ACUMULADOSMES_H

#include <QtGui>
#include "ui_acumuladosmes.h"



class acumuladosmes : public QDialog {
    Q_OBJECT
      public:
                acumuladosmes();
                void pasacodigocuenta(QString codigo,QDate fecha);
      private:
                Ui::acumuladosmes ui;
                bool comadecimal,decimales;
                void cargadatos();
                void ctaexpandepunto();
                void dibujagrafsaldos();
                void dibujagrafsumadebe();
                void dibujagrafsumahaber();
                void resetdatos();
                void generalatex();
      private slots:
                void botonbuscasubcuentapulsado();
                void subcuentacambiada();
                void finedicsubcuenta();
                void refrescar();
                void subcuentaprevia();
                void subcuentasiguiente();
                void txtexport();
                void imprimir();
                void ejercicio_cambiado();

};



#endif 
