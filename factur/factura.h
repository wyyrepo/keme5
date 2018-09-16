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

#ifndef FACTURA_H
#define FACTURA_H

#include <QtGui>
#include "ui_factura.h"



class factura : public QDialog {
    Q_OBJECT
      public:
                factura();
                void pasa_cabecera_doc(QString serie,
                                       QString numero,
                                       QString cuenta,
                                       QDate fecha_fac,
                                       QDate fecha_op,
                                       bool contabilizable,
                                       bool con_ret,
                                       bool re,
                                       QString tipo_ret,
                                       QString retencion,
                                       QString tipo_doc,
                                       QString notas,
                                       QString pie1,
                                       QString pie2
                                       );
                void solo_cabecera_doc(QString serie,
                                       QString numero,
                                       QString cuenta,
                                       QDate fecha_fac,
                                       QDate fecha_op,
                                       bool contabilizable,
                                       bool con_ret,
                                       bool re,
                                       QString tipo_ret,
                                       QString retencion,
                                       QString tipo_doc
                                       );
                void carga_lineas_doc(QString serie,
                                       QString numero);
                void addnotas(QString notas);
                void activa_consulta();
                void pasa_fact_predef(QString codigo, QString descrip,
                                      QString serie,
                                       bool contabilizable,
                                       bool con_ret,
                                       bool re,
                                       QString tipo_ret,
                                       QString tipo_doc,
                                       QString notas,
                                       QString pie1,
                                       QString pie2
                                       );

      private:
                Ui::factura ui;
                QString descripfact;
                QString seriefact;
                QString pie1;
                QString pie2;
                QString moneda;
                QString codigo_moneda;
                QStringList codigoiva;
                QList<double> tipoiva, tipore, baseimponible;
                bool modo_consulta;
                int tipo_operacion;
                bool contabilizable;
                bool coma,decimales;
                bool predefinida;
                QString codigopredef;
                void actualizatotales();
                int ultimafila();
                void latexdoc(QString serie, QString numero);
                void terminar(bool impri);
                QString ltipoiva(QString codigo);
                QString ltipore(QString codigo);
                void fijatipoiva(QString codigo, double tporuno);
                void fijatipore(QString codigo, double tporuno);
                void guarda_predef();

      private slots:
                void actudoc();
                void contenidocambiado(int fila,int columna);
                void posceldacambiada ( int row , int col, int previousRow, int previousColumn );
                void ctabasecambiada();
                void ctabasefinedicion();
                void buscactacli();
                void datoscuenta();
                void ret_cambiado();
                void aceptar();
                void gen_pdf();
                void borralinea();
                void muevearriba();
                void mueveabajo();


};



#endif 
