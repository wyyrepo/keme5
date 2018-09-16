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

#include "sustcuenta.h"
#include "funciones.h"
#include <QSqlQuery>
#include "buscasubcuenta.h"
#include <QMessageBox>

sustcuenta::sustcuenta() : QDialog() {
 ui.setupUi(this);

   connect(ui.buscapushButton1,SIGNAL(clicked()),this,SLOT(buscasubcuenta1()));
   connect(ui.buscapushButton2,SIGNAL(clicked()),this,SLOT(buscasubcuenta2()));
   connect(ui.origenlineEdit,SIGNAL(editingFinished()),this,SLOT(origenexpandepunto()));
   connect(ui.porlineEdit,SIGNAL(editingFinished()),this,SLOT(porexpandepunto()));
   connect(ui.procesarpushButton,SIGNAL(clicked()),this,SLOT(procesar()));

}


void sustcuenta::buscasubcuenta1()
{
    QString cadena2;
    buscasubcuenta *labusqueda=new buscasubcuenta(ui.origenlineEdit->text());
    labusqueda->exec();
    cadena2=labusqueda->seleccioncuenta();
    if (cadena2.length()==anchocuentas()) ui.origenlineEdit->setText(cadena2);
       else ui.origenlineEdit->setText("");
    delete labusqueda;

}


void sustcuenta::buscasubcuenta2()
{
    QString cadena2;
    buscasubcuenta *labusqueda=new buscasubcuenta(ui.porlineEdit->text());
    labusqueda->exec();
    cadena2=labusqueda->seleccioncuenta();
    if (cadena2.length()==anchocuentas()) ui.porlineEdit->setText(cadena2);
       else ui.porlineEdit->setText("");
    delete labusqueda;

}


void sustcuenta::origenexpandepunto()
{
  if (ui.origenlineEdit->text().length()==0) return;
  ui.origenlineEdit->setText(expandepunto(ui.origenlineEdit->text(),anchocuentas()));
  if (ui.origenlineEdit->text().length()<anchocuentas()) buscasubcuenta1();

}


void sustcuenta::porexpandepunto()
{
  if (ui.porlineEdit->text().length()==0) return;
  ui.porlineEdit->setText(expandepunto(ui.porlineEdit->text(),anchocuentas()));
  if (ui.porlineEdit->text().length()<anchocuentas()) buscasubcuenta2();

}

void sustcuenta::procesar()
{
  if (!existesubcuenta(ui.origenlineEdit->text()) || !existesubcuenta(ui.porlineEdit->text()))
      {
       QMessageBox::warning( this, tr("Sustituir subcuentas"),tr("Códigos erróneos"));
       return;
      }
  cambiacuentaconta(ui.origenlineEdit->text(), ui.porlineEdit->text());
  accept();
}

