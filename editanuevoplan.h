/* ----------------------------------------------------------------------------------
    KEME-Contabilidad 2.1; aplicación para llevar contabilidades

    Copyright (C) 2007  José Manuel Díez Botella

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

#ifndef EDITANUEVOPLAN_H
#define EDITANUEVOPLAN_H

#include <QtGui>
#include "ui_editanuevoplan.h"
#include "arbol.h"

class editanuevoplan : public QDialog {
    Q_OBJECT
	public:
                editanuevoplan();
                void insertacuentaarbol(QString cadena1,QString cadena2);
                void cargaarbol();
        private:
                Ui::editanuevoplan ui;
                TreeModel modeloarbol;
        private slots :
                void codigocambiado(QString codigo);
                void codigofinedicion();
                void filaclicked(QModelIndex indice);
                void descripcambiada();
                void botonaceptar();
                void botonborrar();

};


#endif 
 
