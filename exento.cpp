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

#include "exento.h"
#include "funciones.h"
#include <QMessageBox>
#include "aux_express.h"
#include "buscasubcuenta.h"
#include "basedatos.h"

exento::exento(bool concomadecimal, bool condecimales) : QDialog() {
    ui.setupUi(this);

    comadecimal=concomadecimal;
    decimales=condecimales;
 cargacombooperaciones();

 connect(ui.CtabaselineEdit,SIGNAL(textChanged(QString)),this,SLOT(ctabasecambiada()));
 connect(ui.CtabaselineEdit,SIGNAL(editingFinished()),this,SLOT(ctabasefinedicion()));
 connect(ui.CtafralineEdit,SIGNAL(textChanged(QString)),this,SLOT(ctafracambiada()));
 connect(ui.CtafralineEdit,SIGNAL(editingFinished()),this,SLOT(ctafrafinedicion()));
 connect(ui.AceptarButton,SIGNAL(clicked()),this,SLOT(botonaceptar()));
 connect(ui.ctabasepushButton,SIGNAL(clicked()),this,SLOT(buscactabase()));
 connect(ui.ctafrapushButton,SIGNAL(clicked()),this,SLOT(buscactafra()));
 connect(ui.CtabaselineEdit,SIGNAL(textChanged(QString)),this,SLOT(chequeabotonaceptar()));
 connect(ui.CtafralineEdit,SIGNAL(textChanged(QString)),this,SLOT(chequeabotonaceptar()));

  connect(ui.FechafradateEdit,SIGNAL(dateChanged(QDate)),this,SLOT(fechafracambiada()));

 }

void exento::fuerzaemitidas()
{
  ui.emitidasradioButton->setChecked(true);
}


void exento::pasadatos( QString cuentabase, QString baseimponible, QString ctafra,
                        QDate qfechafra, QDate qfechaop,
                                    QString qclaveop,
                                    QString rectificativa,QString frectif,  QString numfact,
                               QString facini, QString facfinal, QString binversion,
                               QString op_no_sujeta, QString cliente, QString cif,
                               QString importacion, QString exportacion,
                               QString exenta_no_deduc)
{
 ui.CtabaselineEdit->setText(cuentabase);
 if (cuentabase.length()>0) ui.basetextLabel->setText(descripcioncuenta(cuentabase));
 ui.baselineEdit->setText(baseimponible);
 ui.CtafralineEdit->setText(ctafra);
 if (ctafra.length()>0) ui.fratextLabel->setText(descripcioncuenta(ctafra));
 ui.FechafradateEdit->setDate(qfechafra);
 ui.fechaoperaciondateEdit->setDate(qfechaop);
 for (int veces=0;veces<ui.clavecomboBox->count();veces++)
     {
        QString cadenaclave=ui.clavecomboBox->itemText(veces);
        QString extracto=cadenaclave.section(" ",0,0);
        extracto=extracto.remove(' ');
        if (extracto==qclaveop)
           {
            ui.clavecomboBox->setCurrentIndex(veces);
            break;
           }
     }

  if (rectificativa=="1") {
     ui.rectificativagroupBox->setChecked(true);
     ui.rectificativalineEdit->setText(frectif);
    }
    else ui.rectificativagroupBox->setChecked(false);

 ui.imp_exentacheckBox->setChecked(importacion=="1");
 ui.exportacionheckBox->setChecked(exportacion=="1");
 ui.exenta_no_deduccheckBox->setChecked(exenta_no_deduc=="1");

 ui.numfacturaslineEdit->setText(numfact);
 ui.iniciallineEdit->setText(facini);
 ui.finallineEdit->setText(facfinal);

 ui.bieninversioncheckBox->setChecked(binversion=="1");
 ui.nosujetacheckBox->setChecked(op_no_sujeta=="1");

 ui.nombrelineEdit->setText(cliente);
 ui.ciflineEdit->setText(cif);
}



void exento::ctabasecambiada()
{
 QMessageBox msgBox;
 QPushButton *aceptarButton = msgBox.addButton(tr("Sí"), QMessageBox::ActionRole);
 msgBox.addButton(tr("No"), QMessageBox::ActionRole);
 msgBox.setWindowTitle(tr("ADVERTENCIA"));
 msgBox.setIcon(QMessageBox::Warning);

    QString cadena;
    if (ui.CtabaselineEdit->text().length()==anchocuentas() && !cod_longitud_variable()) {
	if (!existecodigoplan(ui.CtabaselineEdit->text(),&cadena)) 
	  {
	 // preguntar si se desea añadir la cuenta, si no o si se cancela lo anterior dejar en blanco
	    QString TextoAviso = tr("Esa cuenta no existe, ¿ Desea añadirla ?");
            msgBox.setText(TextoAviso);
            msgBox.exec();
	    if (msgBox.clickedButton() == aceptarButton) 
	         {
                   aux_express *lasubcuenta2=new aux_express();
		   lasubcuenta2->pasacodigo(ui.CtabaselineEdit->text());
		   lasubcuenta2->exec();
	           delete(lasubcuenta2);
	           if (!existecodigoplan(ui.CtabaselineEdit->text(),&cadena))
			ui.CtabaselineEdit->setText("");
		 }
		  else
	            ui.CtabaselineEdit->setText("");	     
	   }
         chequeabotonaceptar();
        }
   if (esauxiliar(ui.CtabaselineEdit->text()))
       ui.basetextLabel->setText(descripcioncuenta(ui.CtabaselineEdit->text()));
     else ui.basetextLabel->setText("");
}

void exento::ctabasefinedicion()
{
  ui.CtabaselineEdit->setText(expandepunto(ui.CtabaselineEdit->text(),anchocuentas()));
 QMessageBox msgBox;
 QPushButton *aceptarButton = msgBox.addButton(tr("Sí"), QMessageBox::ActionRole);
 msgBox.addButton(tr("No"), QMessageBox::ActionRole);
 msgBox.setWindowTitle(tr("ADVERTENCIA"));
 msgBox.setIcon(QMessageBox::Warning);
 QString cadena;
  if (ui.CtabaselineEdit->text().length()>3 && cod_longitud_variable()) {
	if (!existecodigoplan(ui.CtabaselineEdit->text(),&cadena)) 
	  {
	 // preguntar si se desea añadir la cuenta, si no o si se cancela lo anterior dejar en blanco
	    QString TextoAviso = tr("Esa cuenta no existe, ¿ Desea añadirla ?");
            msgBox.setText(TextoAviso);
            msgBox.exec();
	    if (msgBox.clickedButton() == aceptarButton) 
	         {
                   aux_express *lasubcuenta2=new aux_express();
		   lasubcuenta2->pasacodigo(ui.CtabaselineEdit->text());
		   lasubcuenta2->exec();
	           delete(lasubcuenta2); 
	           if (!esauxiliar(ui.CtabaselineEdit->text()))
			ui.CtabaselineEdit->setText("");	     
		 }
		  else
	            ui.CtabaselineEdit->setText("");	     
            chequeabotonaceptar();
	   }
        if (esauxiliar(ui.CtabaselineEdit->text()))
          ui.basetextLabel->setText(descripcioncuenta(ui.CtabaselineEdit->text()));
         else ui.basetextLabel->setText("");
        }
}


void exento::ctafracambiada()
{
 QMessageBox msgBox;
 QPushButton *aceptarButton = msgBox.addButton(tr("Sí"), QMessageBox::ActionRole);
 msgBox.addButton(tr("No"), QMessageBox::ActionRole);
 msgBox.setWindowTitle(tr("ADVERTENCIA"));
 msgBox.setIcon(QMessageBox::Warning);

   QString cadena;
    if (ui.CtafralineEdit->text().length()==anchocuentas() && !cod_longitud_variable()) {
	if (!existecodigoplan(ui.CtafralineEdit->text(),&cadena)) 
	  {
	 // preguntar si se desea añadir la cuenta, si no o si se cancela lo anterior dejar en blanco
	    QString TextoAviso = tr("Esa cuenta no existe, ¿ Desea añadirla ?");
            msgBox.setText(TextoAviso);
            msgBox.exec();
	    if (msgBox.clickedButton() == aceptarButton) 
	         {
                   aux_express *lasubcuenta2=new aux_express();
		   lasubcuenta2->pasacodigo(ui.CtafralineEdit->text());
		   lasubcuenta2->exec();
	           delete(lasubcuenta2); 
	           if (!existecodigoplan(ui.CtafralineEdit->text(),&cadena))
			ui.CtafralineEdit->setText("");	     
		 }
		  else
	            ui.CtafralineEdit->setText("");	     
	}
        }
    chequeabotonaceptar();
    if (esauxiliar(ui.CtafralineEdit->text())) 
	ui.fratextLabel->setText(descripcioncuenta(ui.CtafralineEdit->text()));
    else ui.fratextLabel->setText("");
}

void exento::ctafrafinedicion()
{
   ui.CtafralineEdit->setText(expandepunto(ui.CtafralineEdit->text(),anchocuentas()));
 QMessageBox msgBox;
 QPushButton *aceptarButton = msgBox.addButton(tr("Sí"), QMessageBox::ActionRole);
 msgBox.addButton(tr("No"), QMessageBox::ActionRole);
 msgBox.setWindowTitle(tr("ADVERTENCIA"));
 msgBox.setIcon(QMessageBox::Warning);
 QString cadena;
   ui.CtafralineEdit->setText(expandepunto(ui.CtafralineEdit->text(),anchocuentas()));
     if (ui.CtafralineEdit->text().length()>3 && cod_longitud_variable()) {
	if (!existecodigoplan(ui.CtafralineEdit->text(),&cadena)) 
	  {
	 // preguntar si se desea añadir la cuenta, si no o si se cancela lo anterior dejar en blanco
	    QString TextoAviso = tr("Esa cuenta no existe, ¿ Desea añadirla ?");
            msgBox.setText(TextoAviso);
            msgBox.exec();
	    if (msgBox.clickedButton() == aceptarButton) 
	         {
                   aux_express *lasubcuenta2=new aux_express();
		   lasubcuenta2->pasacodigo(ui.CtafralineEdit->text());
		   lasubcuenta2->exec();
	           delete(lasubcuenta2);
	           if (!esauxiliar(ui.CtafralineEdit->text()))
			ui.CtafralineEdit->setText("");
		 }
		  else
	            ui.CtafralineEdit->setText("");
            chequeabotonaceptar();
	  }
        if (esauxiliar(ui.CtafralineEdit->text()))
	   ui.fratextLabel->setText(descripcioncuenta(ui.CtafralineEdit->text()));
          else ui.fratextLabel->clear();
        }

}


void exento::botonaceptar()
{     
    bool ok;
    double valor=convapunto(ui.baselineEdit->text()).toDouble(&ok);
    if ((valor<0.0001 && valor>-0.0001) || !ok)
	        {
	          QMessageBox::warning( this, "ERROR",
                    tr("El valor de la base no puede ser nulo"));
	          return;
                }


    if (!esauxiliar(ui.CtabaselineEdit->text()))
            {
	      if (ui.CtabaselineEdit->text().length()==0)
	        {
	          QMessageBox::warning( this, "ERROR",
                    tr("La cuenta base no es correcta"));
	          return;
                }
	      if (ui.CtabaselineEdit->text().length()!=0)
	        {
	         QMessageBox::warning( this, "ERROR",
                    tr("La cuenta base no es correcta"));
	         return;
                }
            }

    if (!esauxiliar(ui.CtafralineEdit->text())) 
            {
	      QMessageBox::warning( this, "ERROR",
               tr("La cuenta de factura no es correcta"));
	      return;
            }

     accept();
}


void exento::recuperadatos( QString *cuentabase, QString *baseimponible,  QString *ctafra,
                            QDate *qfechafra, QDate *qfechaop,
                            QString *qclaveop, QString *rectificativa, QString *frectif,
                                    QString *numfact,
                                    QString *facini, QString *facfinal, QString *bieninversion,
                                    QString *op_no_sujeta, QString *nombre, QString *cif,
                                    QString *importacion, QString *exportacion,
                                    QString *exenta_no_deduc)
{
*cuentabase=ui.CtabaselineEdit->text();
*baseimponible=convapunto(ui.baselineEdit->text());
*ctafra=ui.CtafralineEdit->text();
*qfechafra=ui.FechafradateEdit->date();
*qfechaop=ui.fechaoperaciondateEdit->date();
QString cadenaclave=ui.clavecomboBox->currentText();
QString extracto=cadenaclave.section(' ',0,0);
*qclaveop=extracto.remove(' ');

if (ui.rectificativagroupBox->isChecked()) *rectificativa="1";
   else *rectificativa="0";

if (ui.rectificativagroupBox->isChecked()) *frectif=ui.rectificativalineEdit->text();
   else *frectif="";
*numfact=ui.numfacturaslineEdit->text();
*facini=ui.iniciallineEdit->text();
*facfinal=ui.finallineEdit->text();
*bieninversion=ui.bieninversioncheckBox->isChecked() ? "1" : "0";
*op_no_sujeta=ui.nosujetacheckBox->isChecked() ? "1" : "0";
*nombre=ui.nombrelineEdit->text();
*cif=ui.ciflineEdit->text();
*importacion=ui.imp_exentacheckBox->isChecked() ? "1" : "";
*exportacion=ui.exportacionheckBox->isChecked() ? "1" : "";
*exenta_no_deduc=ui.exenta_no_deduccheckBox->isChecked() ? "1" : "";
}


void exento::chequeabotonaceptar()
{
    // QString cadena="";
    if (!esauxiliar(ui.CtabaselineEdit->text()))
        { 
	if (ui.CtabaselineEdit->text().length()!=0)
       { ui.AceptarButton->setEnabled(false); return; }
	if (ui.CtabaselineEdit->text().length()==0)
       { ui.AceptarButton->setEnabled(false); return; }
        }

    if (!esauxiliar(ui.CtafralineEdit->text()))
        { ui.AceptarButton->setEnabled(false); return; }

    ui.AceptarButton->setEnabled(true);

}

void exento::buscactabase()
{
    QString cadena2;
    buscasubcuenta *labusqueda=new buscasubcuenta(ui.CtabaselineEdit->text());
    labusqueda->exec();
    cadena2=labusqueda->seleccioncuenta();
    if (cadena2.length()>0) ui.CtabaselineEdit->setText(cadena2);
       else ui.CtabaselineEdit->setText("");
    delete(labusqueda);
}

void exento::buscactafra()
{
    QString cadena2;
    buscasubcuenta *labusqueda=new buscasubcuenta(ui.CtafralineEdit->text());
    labusqueda->exec();
    cadena2=labusqueda->seleccioncuenta();
    if (cadena2.length()>0) ui.CtafralineEdit->setText(cadena2);
       else ui.CtafralineEdit->setText("");
    delete(labusqueda);
}

bool exento::eslibroemitidas()
{
 return ui.emitidasradioButton->isChecked();
}

void exento::cargacombooperaciones()
{
  QStringList lista=basedatos::instancia()->listaoperaciones_expedidas();
  QStringList lista2=basedatos::instancia()->listaoperaciones_recibidas();
  for (int veces=0;veces<lista2.count();veces++)
     {
        QString cadenaclave=lista2.at(veces);
        QString extracto=cadenaclave.section(" ",0,0);
        extracto=extracto.remove(' ');
        int veces2;
        for (veces2=0;veces2<lista.count();veces2++)
           {
            QString extracto2=lista.at(veces2).section(" ",0,0);
            if (extracto2==extracto) break;
           }
        if (veces2>=lista.count())
            // añadimos cadenaclave a lista
            lista << cadenaclave;
     }
  lista.sort();
  ui.clavecomboBox->addItems (lista);

}

void exento::fechafracambiada()
{
 ui.fechaoperaciondateEdit->setDate(ui.FechafradateEdit->date());
}


void exento::modoconsulta()
{
  ui.baselineEdit->setReadOnly(true);
  ui.bieninversioncheckBox->setEnabled(false);
  ui.imp_exentacheckBox->setEnabled(false);
  ui.exportacionheckBox->setEnabled(false);
  ui.exenta_no_deduccheckBox->setEnabled(false);
  ui.clavecomboBox->setEnabled(false);
  ui.CtabaselineEdit->setReadOnly(true);
  ui.ctabasepushButton->setEnabled(false);
  ui.ctabasepushButton->setEnabled(false);
  ui.CtafralineEdit->setReadOnly(true);
  ui.ctafrapushButton->setEnabled(false);
  ui.emitidasradioButton->setEnabled(false);
  ui.FechafradateEdit->setReadOnly(true);
  ui.fechaoperaciondateEdit->setReadOnly(true);
  ui.finallineEdit->setReadOnly(true);
  ui.iniciallineEdit->setReadOnly(true);
  ui.nosujetacheckBox->setEnabled(false);
  ui.numfacturaslineEdit->setReadOnly(true);
  ui.recibidasradioButton->setEnabled(false);
  ui.rectificativalineEdit->setReadOnly(true);
  ui.rectificativagroupBox->setEnabled(false);
  ui.nombrelineEdit->setReadOnly(true);
  ui.ciflineEdit->setReadOnly(true);

}
