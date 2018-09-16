/* ----------------------------------------------------------------------------------
    KEME-Contabilidad; aplicación para llevar contabilidades

    Copyright (C)

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

#ifndef WIDGETBARRAS_H
#define WIDGETBARRAS_H

#include <QtGui>
#include "ui_WidgetBarras.h"


class WidgetBarras : public QWidget {
    Q_OBJECT
      public:
        WidgetBarras(QString nombreEmpresa,QString ejercicio,
                    QString titulo1,QString titulo2,
                    QStringList etiquetas_parte1,
                    QStringList etiquetas_parte2,
                    QList<double> valores_parte1,
                    QList<double> valores_parte2,
			        QWidget * parent);
        void mostrar(QString nombreEmpresa,QString ejercicio,
                    QString titulo1,QString titulo2,
                    QStringList etiquetas_parte1,
                    QStringList etiquetas_parte2,
                    QList<double> valores_parte1,
                    QList<double> valores_parte2);
        ~WidgetBarras();

      private:
        Ui::WidgetBarras ui;
        QGraphicsScene * graphicsscene;
        void etiquetasbasicas(QString nombreEmpresa,QString ejercicio);
        void comprobarvalidez(QList<double> valores,double *total,bool *negativo);
        static void establecercaja(QGraphicsRectItem * caja,int i);
        static void establecertexto(QGraphicsTextItem * texto,int x, int y, int i);
        void dibujarcolumna(QStringList etiquetas, QList<double> valores, int posicionx, double total, int posicionetiquetasx, int desplazamiento);

      private slots:
        void guardarimagen();
};

#endif
