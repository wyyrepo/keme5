/* ----------------------------------------------------------------------------------
    KEME-Contabilidad; aplicación para llevar contabilidades

    Copyright (C)   José Manuel Díez Botella

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

#ifndef GRAFICO_H
#define GRAFICO_H

#include <QtGui>
#include "WidgetBarras.h"
#include <QDialog>

class grafico : public QDialog {
    Q_OBJECT
      public:
        grafico (QString nombreEmpresa,QString ejercicio,
                QString titulo1,QString titulo2,
                QStringList etiquetas_parte1,
                QStringList etiquetas_parte2,
                QList<double> valores_parte1,
                QList<double> valores_parte2,
                QString titulo);
      private:
        WidgetBarras * wbarras;
};

#endif 
