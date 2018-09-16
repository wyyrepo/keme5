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

#include "edita_ref.h"
#include "basedatos.h"
#include "funciones.h"
#include "buscasubcuenta.h"

edita_ref::edita_ref() : QDialog() {
    ui.setupUi(this);

    QString clavedefecto=clave_iva_defecto();
    double tipo,re;
    QString qdescrip;
    if (clavedefecto.length()>0 && existeclavetipoiva(clavedefecto,&tipo,&re,&qdescrip))
       {
        ui.ivacomboBox->insertItem ( 0,clavedefecto) ;
        ui.ivarecomboBox->insertItem ( 0,clavedefecto) ;
       }


    QSqlQuery query = basedatos::instancia()->selectTodoTiposivanoclave(clavedefecto);

    if ( query.isActive() )
       {
         while (query.next())
            {
             ui.ivacomboBox->insertItem ( -1,query.value(0).toString()) ;
             ui.ivarecomboBox->insertItem ( -1,query.value(0).toString()) ;
            }
         }
    ui.ivacomboBox->insertItem(-1,tr("EXENTO"));
    ui.ivarecomboBox->insertItem(-1,tr("EXENTO"));
    coma=haycomadecimal();
    decimales=haydecimales();

    connect(ui.aceptarpushButton, SIGNAL(clicked()), SLOT(terminar()));
    connect(ui.ivacomboBox,SIGNAL(currentIndexChanged(int)), SLOT(ivacombocambiado()));
    connect(ui.ivarecomboBox,SIGNAL(currentIndexChanged(int)), SLOT(ivarecombocambiado()));
    connect(ui.pvplineEdit,SIGNAL(editingFinished()),SLOT(pvpfinedic()));
    connect(ui.ctapushButton,SIGNAL(clicked()),this,SLOT(buscacta()));
    connect(ui.cuentalineEdit,SIGNAL(textChanged(QString)),this,SLOT(cuentacambiada()));
    connect(ui.cuentalineEdit,SIGNAL(editingFinished()),this,SLOT(ctaexpandepunto()));
    ivacombocambiado();
    ivarecombocambiado();
}

void edita_ref::pasacodigo(QString codigo)
{
    QString descrip, tipoiva, tipoivare, cuenta;
    double pvp;
    basedatos::instancia()->carga_ref(codigo,
                                   &descrip,
                                   &tipoiva,
                                   &tipoivare,
                                   &cuenta,
                                   &pvp);
    ui.codigolineEdit->setText(codigo);
    ui.descriplineEdit->setText(descrip);
    ui.cuentalineEdit->setText(cuenta);

    for (int veces=0; veces<ui.ivacomboBox->count(); veces++)
        if (ui.ivacomboBox->itemText(veces)==tipoiva)
           {
             ui.ivacomboBox->setCurrentIndex(veces);
             break;
           }

    for (int veces=0; veces<ui.ivarecomboBox->count(); veces++)
        if (ui.ivarecomboBox->itemText(veces)==tipoivare)
           {
             ui.ivarecomboBox->setCurrentIndex(veces);
             break;
           }

    ui.pvplineEdit->setText(formateanumero(pvp,coma, decimales));
}


void edita_ref::terminar()
{
    basedatos::instancia()->guarda_ref(ui.codigolineEdit->text(),
                                   ui.descriplineEdit->text(),
                                   ui.ivacomboBox->currentText(),
                                   ui.ivarecomboBox->currentText(),
                                   ui.cuentalineEdit->text(),
                                   convapunto(ui.pvplineEdit->text()));
    accept();
}

void edita_ref::ivacombocambiado()
{
    QString tipo,re;
    QString descrip;
    existeclaveiva(ui.ivacomboBox->currentText(),
                       &tipo,&re,&descrip);

    ui.descripIVAlabel->setText(descrip);

}

void edita_ref::ivarecombocambiado()
{
    QString tipo,re;
    QString descrip;

    existeclaveiva(ui.ivarecomboBox->currentText(),
                       &tipo,&re,&descrip);

    ui.descripIVARElabel->setText(descrip);

}

void edita_ref::pvpfinedic()
{
    QString precio=ui.pvplineEdit->text();
    ui.pvplineEdit->setText(coma ? convacoma(precio) : convapunto(precio));
}


void edita_ref::buscacta()
{
    QString cadena2;
    buscasubcuenta *labusqueda=new buscasubcuenta(ui.cuentalineEdit->text());
    int cod=labusqueda->exec();
    cadena2=labusqueda->seleccioncuenta();
    if (cod==QDialog::Accepted && existesubcuenta(cadena2))
      ui.cuentalineEdit->setText(cadena2);
       else ui.cuentalineEdit->setText("");
    delete labusqueda;

}


void edita_ref::cuentacambiada()
{
    if(existesubcuenta(ui.cuentalineEdit->text()))
       {
        ui.aceptarpushButton->setEnabled(true);
        ui.descripctalabel->setText(descripcioncuenta(ui.cuentalineEdit->text()));
       }
     else
         {
          ui.aceptarpushButton->setEnabled(false);
          ui.descripctalabel->clear();
         }
}


void edita_ref::ctaexpandepunto()
{
  ui.cuentalineEdit->setText(expandepunto(ui.cuentalineEdit->text(),anchocuentas()));
}

