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

#include "libro_recibidas.h"
#include <QSqlQuery>
#include "funciones.h"
#include "basedatos.h"
#include "tabla_iva_sop.h"
#include <QMessageBox>
#include "buscasubcuenta.h"


libro_recibidas::libro_recibidas(QString qusuario) : QDialog() {
    ui.setupUi(this);


usuario=qusuario;
  QSqlQuery query = basedatos::instancia()->selectCodigoejerciciosordercodigo();
  QStringList ej1;

  if ( query.isActive() ) {
          while ( query.next() )
                ej1 << query.value(0).toString();
	  }
  ui.ejerciciocomboBox->addItems(ej1);

  comadecimal=haycomadecimal(); decimales=haydecimales();

  connect(ui.consultarpushButton,SIGNAL( clicked()),this,SLOT(consultatex()));
  connect(ui.imprimirpushButton,SIGNAL( clicked()),this,SLOT(imprime()));
  connect(ui.copiarpushButton,SIGNAL( clicked()),this,SLOT(copiar()));

  connect(ui.prorratacheckBox,SIGNAL(stateChanged(int)),SLOT(prorratacambiado()));

  connect(ui.bicheckBox ,SIGNAL(stateChanged(int)),SLOT(bicheckcambiado()));
  connect(ui.latexpushButton,SIGNAL(clicked()),SLOT(latex()));

  connect(ui.tablapushButton,SIGNAL(clicked()),SLOT(tabla()));

  connect(ui.codigopushButton,SIGNAL(clicked(bool)),SLOT(boton_cuenta_pulsado()));
  connect(ui.cuentalineEdit,SIGNAL(editingFinished()),this,SLOT(cuenta_finedicion()));
  connect(ui.cuentalineEdit,SIGNAL(textChanged(QString)),this,SLOT(cuenta_cambiada()));

  ui.inicialdateEdit->setDate(QDate::currentDate());
  ui.finaldateEdit->setDate(QDate::currentDate());

  ordenrecepcion=basedatos::instancia()->hay_secuencia_recibidas();

#ifdef NOEDITTEX
 ui.latexpushButton->hide();
#endif


}

void libro_recibidas::prorratacambiado()
{
 if (ui.prorratacheckBox->isChecked())
    {
     ui.horizontalradioButton->setChecked(true);
     ui.verticalradioButton->setChecked(false);
     ui.groupBox->setEnabled(false);
    }
    else
        {
         ui.horizontalradioButton->setChecked(false);
         ui.verticalradioButton->setChecked(true);
         ui.groupBox->setEnabled(true);
        }
}


void libro_recibidas::bicheckcambiado()
{
    ui.prorratacheckBox->disconnect(SIGNAL(stateChanged(int)));

    if (ui.bicheckBox->isChecked())
       {
        ui.horizontalradioButton->setChecked(true);
        ui.verticalradioButton->setChecked(false);
        ui.groupBox->setEnabled(false);
        ui.prorratacheckBox->setChecked(false);
        ui.prorratacheckBox->setEnabled(false);
       }
       else
           {
            ui.horizontalradioButton->setChecked(false);
            ui.verticalradioButton->setChecked(true);
            ui.groupBox->setEnabled(true);
            ui.prorratacheckBox->setChecked(false);
            ui.prorratacheckBox->setEnabled(true);
           }
     connect(ui.prorratacheckBox,SIGNAL(stateChanged(int)),SLOT(prorratacambiado()));
}

bool libro_recibidas::generalatexsoportado()
{
   if (!ui.interiorescheckBox->isChecked() && !ui.aibcheckBox->isChecked() && 
       !ui.autofacturascheckBox->isChecked() && !ui.rectificativascheckBox->isChecked()
       && !ui.autofacturasnouecheckBox->isChecked() && !ui.agrariocheckBox->isChecked()
       && !ui.isp_op_interiorescheckBox->isChecked() && !ui.importacionescheckBox->isChecked())
       return false;

   QString qfichero=dirtrabajo();
   qfichero.append(QDir::separator());
   qfichero+=nombrefichero();
   qfichero+=".tex";
   QFile fichero(adapta(qfichero));
   if (! fichero.open( QIODevice::WriteOnly ) ) return false;

    QTextStream stream( &fichero );
    stream.setCodec("UTF-8");
    stream << cabeceralatex();
    stream << margen_extra_latex();

    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {";
    stream << filtracad(nombreempresa()) << "}}";
    stream << "\\end{center}";

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    if (!cuenta.isEmpty())
     {
      stream << "\\begin{center}" << "\n";
      stream << "{\\Large \\textbf {";
      stream << filtracad(cuenta) + " " + descripcioncuenta(cuenta) << "}}";
      stream << "\\end{center}";
     }


    stream << "\\begin{center}" << "\n";
    stream << "\\begin{longtable}{|r|p{1.5cm}|c|c|c|p{3cm}|r|r|r|r|r|}" << "\n";
    stream << "\\hline" << "\n";

   stream << "\\multicolumn{11}{|c|} {\\textbf{";
   QString cadena=filtracad(ui.cabeceralineEdit->text());
   // --------------------------------------------------------------------------------------
   stream << cadena;
   stream <<  "}} \\\\";
   stream << "\\hline" << "\n";
    // -------------------------------------------------------------------------------------
    if (ordenrecepcion) stream << "{\\tiny{" << tr("Recep.") << "}} & ";
        else
            stream << "{\\tiny{" << tr("Orden") << "}} & ";
    stream << "{\\tiny{" << tr("Factura") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha Fra.") << "}} & ";
    stream << "{\\tiny{" << tr("Contab.") << "}} & ";
    stream << "{\\tiny{" << tr("CIF/NIF") << "}} & ";
    stream << "{\\tiny{" << tr("Proveedor/acreedor") << "}} & ";
    // stream << "{\\tiny{" << tr("CIF/NIF") << "}} & ";
    stream << "{\\tiny{" << tr("Asto.") << "}} & ";
    stream << "{\\tiny{" << tr("Base Imp.") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo") << "}} & ";
    stream << "{\\tiny{" << tr("Cuota") << "}} & ";
    stream << "{\\tiny{" << tr("Total") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endfirsthead";
    // ----------------------------------------------------------------------------------------
    stream << "\\hline" << "\n";
    // stream << tr(" \\\\") << "\n";
    if (ordenrecepcion) stream << "{\\tiny{" << tr("Recep.") << "}} & ";
        else
          stream << "{\\tiny{" << tr("Orden") << "}} & ";
    stream << "{\\tiny{" << tr("Factura") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha Fra.") << "}} & ";
    stream << "{\\tiny{" << tr("Contab.") << "}} & ";
    stream << "{\\tiny{" << tr("CIF/NIF") << "}} & ";
    stream << "{\\tiny{" << tr("Proveedor/acreedor") << "}} & ";
    // stream << "{\\tiny{" << tr("CIF/NIF") << "}} & ";
    stream << "{\\tiny{" << tr("Asto.") << "}} & ";
    stream << "{\\tiny{" << tr("Base Imp.") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo") << "}} & ";
    stream << "{\\tiny{" << tr("Cuota") << "}} & ";
    stream << "{\\tiny{" << tr("Total") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endhead" << "\n";
    // ---------------------------------------------------------------------------------------

    QString ejercicio=ui.ejerciciocomboBox->currentText();
    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    int registros = basedatos::instancia()->num_registros_recibidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(),false,cuenta);

   // ÚLTIMOS PARÁMETROS
   // bool filtrarbinversion, bool solobi, bool sinbi,
   // bool filtrarnosujetas, bool solonosujetas, bool sinnosujetas


    QSqlQuery query =
      basedatos::instancia()->registros_recibidas(ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(), cuenta,
         ui.orden_fecha_faccheckBox->isChecked());

    ui.progressBar->setMaximum(registros);
    int numorden=0;
    int numprog=1;
    QString cadnum,guardadoc,guardaprov;
    double base=0;
    double cuota=0;
    if ( query.isActive() ) {
          while ( query.next() )
              {
                bool esautofactura=query.value(10).toBool() || query.value(11).toBool()
                                   || query.value(16).toBool();
	        ui.progressBar->setValue(numprog);
                if (guardadoc!=query.value(0).toString() || query.value(0).toString().length()==0 
                    || numprog==1 || guardaprov!=query.value(2).toString()) numorden++; 
                guardadoc=query.value(0).toString();
                guardaprov=query.value(2).toString();
                stream << "{\\tiny " <<
                        (ordenrecepcion ? query.value(15).toString(): cadnum.setNum(numorden))
                        << "} & {\\tiny ";
	        stream << filtracad(query.value(0).toString()) << "} & {\\tiny "; // fra.
	        stream << query.value(9).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha fra
	        stream << query.value(1).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha contable
                if (!esautofactura)
                  {
                    stream << ( query.value(14).toString().isEmpty() ?
                                filtracad(cifcta(query.value(2).toString())) :
                                filtracad(query.value(14).toString()))
                                << "} & {\\tiny "; // cif
                   // stream <<  query.value(2).toString() << "} & {\\tiny "; // cuenta
                   stream <<
                           (query.value(13).toString().isEmpty() ?
                           filtracad(query.value(3).toString()) :
                           filtracad(query.value(13).toString())) << "} & {\\tiny "; // proveedor
	           //if (aib || eib) stream <<  nifcomunitariocta(query.value(2).toString()) << "} & {\\tiny "; // cif
		   //else stream <<  cifcta(query.value(2).toString()) << "} & {\\tiny "; // cif
                  }
                  else
                      {
                       stream <<  basedatos::instancia()->cif() << "} & {\\tiny "; // cif
                       // stream << "} & {\\tiny "; // nada de cuenta
	               stream <<  filtracad(nombreempresa()) << "} & {\\tiny "; // proveedor
                      }
                stream <<  query.value(4).toString() << "} & {\\tiny "; // asiento
                stream <<  formatea_redondeado_sep(query.value(5).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // base iva
                stream <<  formatea_redondeado_sep(query.value(6).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // tipo
                stream <<  formatea_redondeado_sep(query.value(7).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // cuota
                stream <<  formatea_redondeado_sep(query.value(8).toDouble(),comadecimal,decimales) << "} \\\\ \n  "
                       << "\\hline\n"; // total
                if (query.value(18).toBool())
                   {
                    // criterio de caja
                    QSqlQuery q=basedatos::instancia()->datos_venci_apunte_iva_caja_liquidados(
                            query.value(19).toString());
                    // v.fecha_vencimiento, v.liq_iva_fecha, v.forzar_liq_iva, v.importe,
                    // v.num, v.medio_realizacion, v.cuenta_bancaria
                    bool vacio=true;
                    if (q.isActive())
                        while (q.next())
                         {
                          vacio=false;
                          stream << "\\multicolumn{2}{|r|} {\\tiny ";
                          stream << tr("Importe pago");
                          stream << "} & {\\tiny ";
                          stream << formatea_redondeado_sep(q.value(3).toDouble(),comadecimal,decimales);
                          stream << "} & {\\tiny ";
                          stream << tr("FECHA:");
                          stream << "} & {\\tiny ";
                          stream << q.value(1).toDate().toString("dd-MM-yyyy");
                          stream << "} & \\multicolumn{2}{|r|} {\\tiny ";
                          stream << q.value(5).toString();
                          stream << "} & {\\tiny ";
                          stream << tr("Cuenta:");
                          stream << "} & \\multicolumn{3}{|l|} {\\tiny ";
                          stream << q.value(6).toString();
                          stream << "} \\\\ \n  "
                                  << "\\hline\n";
                         }
                    if (vacio)
                       {
                        stream << "\\multicolumn{2}{|r|} {\\tiny ";
                        stream << tr("Importe pago");
                        stream << "} & {\\tiny ";
                        stream << "} & {\\tiny ";
                        stream << tr("FECHA:");
                        stream << "} & {\\tiny ";
                        stream << "} & \\multicolumn{2}{|r|} {\\tiny ";
                        stream << "} & {\\tiny ";
                        stream << tr("Cuenta:");
                        stream << "} & \\multicolumn{3}{|l|} {\\tiny ";
                        stream << "} \\\\ \n  "
                             << "\\hline\n";
                       }
                   }
                numprog++;
	        base+=query.value(5).toDouble();
	        cuota+=query.value(7).toDouble();
	      }
      }

    stream << "\\multicolumn{7}{|r|} {\\textbf{";
   // --------------------------------------------------------------------------------------
    stream << tr("SUMAS TOTALES:") << "}} ";
    stream << " & {\\tiny " << formatea_redondeado_sep(base,comadecimal,decimales) << "} & & {\\tiny ";
    stream << formatea_redondeado_sep(cuota,comadecimal,decimales) << "} &  {\\tiny ";
    stream << formatea_redondeado_sep(base+cuota,comadecimal,decimales);
    stream <<  "} \\\\ \n";
    stream << "\\hline" << "\n";


    stream << "\\end{longtable}" << "\n";
    stream << "\\end{center}" << "\n";

    stream << "% FIN_CUERPO\n";
    stream << "\\end{document}" << "\n";
    
    fichero.close();

    ui.progressBar->reset();

    return true;

}





void libro_recibidas::consultatex()
{

   if (ui.prorratacheckBox->isChecked())
      {
       if (!generalatexprorrata())
          return;
      }
     else
       {
        if (!ui.horizontalradioButton->isChecked())
          {
           if  (!generalatexsoportado())
              return;
          }

        if (ui.horizontalradioButton->isChecked() && !ui.bicheckBox->isChecked())
          {
            if  (!generalatexsoportado_horizontal())
            return;
          }

        if (ui.horizontalradioButton->isChecked() && ui.bicheckBox->isChecked())
          {
            if  (!generalatexsoportado_horizontal_bi())
            return;
          }
       }

   int valor=consultalatex2(nombrefichero());
   if (valor==1)
       QMessageBox::warning( this, tr("Libros de FACTURAS"),tr("PROBLEMAS al llamar a Latex"));
   if (valor==2)
       QMessageBox::warning( this, tr("Libros de FACTURAS"),
                             tr("PROBLEMAS al llamar a ")+visordvi());


}


void libro_recibidas::imprime()
{
   if (ui.prorratacheckBox->isChecked())
      {
       if (!generalatexprorrata())
          return;
      }
     else
       {
        if (!ui.horizontalradioButton->isChecked())
          {
           if  (!generalatexsoportado())
            return;
          }
        if (ui.horizontalradioButton->isChecked())
         {
          if  (!generalatexsoportado_horizontal())
          return;
         }
       }
   int valor=imprimelatex2(nombrefichero());
   if (valor==1)
       QMessageBox::warning( this, tr("Libros de IVA"),tr("PROBLEMAS al llamar a Latex"));
   if (valor==2)
       QMessageBox::warning( this, tr("Libros de IVA"),
                                tr("PROBLEMAS al llamar a 'dvips'"));
   if (valor==3)
       QMessageBox::warning( this, tr("Libros de IVA"),
                             tr("PROBLEMAS al llamar a ")+programa_imprimir());

}


QString libro_recibidas::nombrefichero()
{
  QString cadena;
  if (!ui.prorratacheckBox->isChecked())
     cadena=tr("facturas_recibidas");
   else cadena=tr("facturas_recibidas_prorrata");
  return cadena;
}


void libro_recibidas::copiar()
{
   if (ui.prorratacheckBox->isChecked())
      {
       copiaprorrata();
       ui.progressBar->reset();
       return;
      }
   else
    {
     if (!ui.horizontalradioButton->isChecked())
      {
       copiavertical();
       ui.progressBar->reset();
       return;
      }

     if (ui.horizontalradioButton->isChecked())
      {
       if (ui.bicheckBox->isChecked()) copiahorizontal_bi();
         else copiahorizontal();
       ui.progressBar->reset();
       return;
      }
    }
}



bool libro_recibidas::generalatexsoportado_horizontal()
{
   if (!ui.interiorescheckBox->isChecked() && !ui.aibcheckBox->isChecked() &&
       !ui.autofacturascheckBox->isChecked()  && !ui.rectificativascheckBox->isChecked() &&
       !ui.autofacturasnouecheckBox->isChecked() && !ui.agrariocheckBox->isChecked() &&
       !ui.isp_op_interiorescheckBox->isChecked() && !ui.importacionescheckBox->isChecked())
       return false;

   QString qfichero=dirtrabajo();
   qfichero.append(QDir::separator());
   qfichero+=nombrefichero();
   qfichero+=".tex";
   QFile fichero(adapta(qfichero));
   if (! fichero.open( QIODevice::WriteOnly ) ) return false;

    QTextStream stream( &fichero );
    stream.setCodec("UTF-8");
    stream << cabeceralatex();
    stream << margen_extra_latex();

    stream << "\\begin{landscape}" << "\n";

    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {";
    stream << filtracad(nombreempresa()) << "}}";
    stream << "\\end{center}";

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    if (!cuenta.isEmpty())
     {
      stream << "\\begin{center}" << "\n";
      stream << "{\\Large \\textbf {";
      stream << filtracad(cuenta) + " " + descripcioncuenta(cuenta) << "}}";
      stream << "\\end{center}";
     }


    stream << "\\begin{center}" << "\n";
    stream << "\\begin{longtable}{|r|p{2cm}|c|c|c|p{5cm}|l|r|r|r|r|r|}" << "\n";
    stream << "\\hline" << "\n";

   stream << "\\multicolumn{12}{|c|} {\\textbf{";
   QString cadena=filtracad(ui.cabeceralineEdit->text());
   // --------------------------------------------------------------------------------------
   stream << cadena;
   stream <<  "}} \\\\";
   stream << "\\hline" << "\n";
    // -------------------------------------------------------------------------------------
   if (ordenrecepcion) stream << "{\\tiny{" << tr("Recep.") << "}} & ";
       else
          stream << "{\\tiny{" << tr("Orden") << "}} & ";
    stream << "{\\tiny{" << tr("Factura") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha Fra.") << "}} & ";
    stream << "{\\tiny{" << tr("Contab.") << "}} & ";
    stream << "{\\tiny{" << tr("Cuenta") << "}} & ";
    stream << "{\\tiny{" << tr("Proveedor/acreedor") << "}} & ";
    stream << "{\\tiny{" << tr("CIF/NIF") << "}} & ";
    stream << "{\\tiny{" << tr("Asto.") << "}} & ";
    stream << "{\\tiny{" << tr("Base Imp.") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo") << "}} & ";
    stream << "{\\tiny{" << tr("Cuota") << "}} & ";
    stream << "{\\tiny{" << tr("Total") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endfirsthead";
    // ----------------------------------------------------------------------------------------
    stream << "\\hline" << "\n";
    // stream << tr(" \\\\") << "\n";
    if (ordenrecepcion) stream << "{\\tiny{" << tr("Recep.") << "}} & ";
        else
           stream << "{\\tiny{" << tr("Orden") << "}} & ";
    stream << "{\\tiny{" << tr("Factura") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha Fra.") << "}} & ";
    stream << "{\\tiny{" << tr("Contab.") << "}} & ";
    stream << "{\\tiny{" << tr("Cuenta") << "}} & ";
    stream << "{\\tiny{" << tr("Proveedor/acreedor") << "}} & ";
    stream << "{\\tiny{" << tr("CIF/NIF") << "}} & ";
    stream << "{\\tiny{" << tr("Asto.") << "}} & ";
    stream << "{\\tiny{" << tr("Base Imp.") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo") << "}} & ";
    stream << "{\\tiny{" << tr("Cuota") << "}} & ";
    stream << "{\\tiny{" << tr("Total") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endhead" << "\n";
    // ---------------------------------------------------------------------------------------

    QString ejercicio=ui.ejerciciocomboBox->currentText();
    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    int registros = basedatos::instancia()->num_registros_recibidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(),false, cuenta);

    QSqlQuery query =
      basedatos::instancia()->registros_recibidas(ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(),cuenta);

    ui.progressBar->setMaximum(registros);
    int numorden=0;
    int numprog=1;
    QString cadnum,guardadoc,guardaprov;
    double base=0;
    double cuota=0;
    if ( query.isActive() ) {
          while ( query.next() )
              {
                bool esautofactura=query.value(10).toBool() || query.value(11).toBool()
                                   ||query.value(16).toBool();
	        ui.progressBar->setValue(numprog);
                if (guardadoc!=query.value(0).toString() || query.value(0).toString().length()==0 
                    || numprog==1 || guardaprov!=query.value(2).toString()) numorden++; 
                guardadoc=query.value(0).toString();
                guardaprov=query.value(2).toString();               
                stream << "{\\tiny " <<
                        (ordenrecepcion ? query.value(15).toString() : cadnum.setNum(numorden))
                        << "} & {\\tiny ";
	        stream << filtracad(query.value(0).toString()) << "} & {\\tiny "; // fra.
	        stream << query.value(9).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha fra
	        stream << query.value(1).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha contable
                if (!esautofactura)
                   {
	            stream <<  query.value(2).toString() << "} & {\\tiny "; // cuenta
                    stream << (query.value(13).toString().isEmpty() ?
                               filtracad(query.value(3).toString()) :
                               filtracad(query.value(13).toString()))
                            << "} & {\\tiny "; // proveedor
                    stream << ( query.value(14).toString().isEmpty() ?
                                filtracad(cifcta(query.value(2).toString())) :
                                filtracad(query.value(14).toString()))
                                << "} & {\\tiny "; // cif
                   }
                   else
                       {
	                stream <<  "} & {\\tiny "; // nada de cuenta
	                stream <<  filtracad(nombreempresa()) << "} & {\\tiny "; // proveedor
                        stream <<  basedatos::instancia()->cif() << "} & {\\tiny "; // cif
                       }

                stream <<  query.value(4).toString() << "} & {\\tiny "; // asiento
                stream <<  formatea_redondeado_sep(query.value(5).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // base iva
                stream <<  formatea_redondeado_sep(query.value(6).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // tipo
                stream <<  formatea_redondeado_sep(query.value(7).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // cuota
                stream <<  formatea_redondeado_sep(query.value(8).toDouble(),comadecimal,decimales) << "} \\\\ \n  "
                       << "\\hline\n"; // total
                if (query.value(18).toBool())
                   {
                    // criterio de caja
                    QSqlQuery q=basedatos::instancia()->datos_venci_apunte_iva_caja_liquidados(
                            query.value(19).toString());
                    // v.fecha_vencimiento, v.liq_iva_fecha, v.forzar_liq_iva, v.importe,
                    // v.num, v.medio_realizacion, v.cuenta_bancaria
                    bool vacio=true;
                    if (q.isActive())
                        while (q.next())
                         {
                          vacio=false;
                          stream << "\\multicolumn{3}{|r|} {\\tiny ";
                          stream << tr("Importe pago");
                          stream << "} & {\\tiny ";
                          stream << formatea_redondeado_sep(q.value(3).toDouble(),comadecimal,decimales);
                          stream << "} & \\multicolumn{2}{|l|} {\\tiny ";
                          stream << q.value(5).toString();
                          stream << "} & {\\tiny ";
                          stream << tr("FECHA:");
                          stream << "} & {\\tiny ";
                          stream << q.value(1).toDate().toString("dd-MM-yyyy");
                          stream << "} & {\\tiny ";
                          stream << tr("Cuenta:");
                          stream << "} & \\multicolumn{3}{|l|} {\\tiny ";
                          stream << q.value(6).toString();
                          stream << "} \\\\ \n  "
                                  << "\\hline\n";
                         }
                    if (vacio)
                       {
                        stream << "\\multicolumn{3}{|r|} {\\tiny ";
                        stream << tr("Importe pago");
                        stream << "} & {\\tiny ";
                        stream << "} & \\multicolumn{2}{|l|} {\\tiny ";
                        stream << "} & {\\tiny ";
                        stream << tr("FECHA:");
                        stream << "} & {\\tiny ";
                        stream << "} & {\\tiny ";
                        stream << tr("Cuenta:");
                        stream << "} & \\multicolumn{3}{|l|} {\\tiny ";
                        stream << "} \\\\ \n  "
                                << "\\hline\n";
                       }
                   }
                numprog++;
	        base+=query.value(5).toDouble();
	        cuota+=query.value(7).toDouble();
	      }
      }

    stream << "\\multicolumn{8}{|r|} {\\textbf{";
   // --------------------------------------------------------------------------------------
    stream << tr("SUMAS TOTALES:") << "}} ";
    stream << " & {\\tiny " << formatea_redondeado_sep(base,comadecimal,decimales) << "} & & {\\tiny ";
    stream << formatea_redondeado_sep(cuota,comadecimal,decimales) << "} &  {\\tiny ";
    stream << formatea_redondeado_sep(base+cuota,comadecimal,decimales);
    stream <<  "} \\\\ \n";
    stream << "\\hline" << "\n";


    stream << "\\end{longtable}" << "\n";
    stream << "\\end{center}" << "\n";

    stream << "\\end{landscape}\n";

    stream << "% FIN_CUERPO\n";
    stream << "\\end{document}" << "\n";

    fichero.close();

    ui.progressBar->reset();

    return true;

}


void libro_recibidas::copiavertical()
{
   QClipboard *cb = QApplication::clipboard();
   QString global;


   global=filtracad(nombreempresa()) + "\n" ;
   global+=ui.cabeceralineEdit->text();
   global+="\n";
   global+="\n";
   global+=(ordenrecepcion ? tr("Recep.") : tr("Orden"))+"\t";
   global+=tr("Factura")+"\t";
   global+=tr("Fecha Fra.")+"\t";
   global+=tr("Contab.")+"\t";
   global+=tr("CIF/NIF")+"\t";
   global+=tr("Proveedor/acreedor")+"\t";
    // tr("CIF/NIF") << "}} & ";
   global+=tr("Asto.")+"\t";
   global+=tr("Base Imp.")+"\t";
   global+=tr("Tipo")+"\t";
   global+=tr("Cuota")+"\t";
   global+=tr("Total")+"\t";
   global+="\n";

    QString ejercicio=ui.ejerciciocomboBox->currentText();
    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    int registros = basedatos::instancia()->num_registros_recibidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(),false,cuenta);

    QSqlQuery query =
      basedatos::instancia()->registros_recibidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(), cuenta);

    ui.progressBar->setMaximum(registros);
    int numorden=0;
    int numprog=1;
    QString cadnum,guardadoc,guardaprov;
    double base=0;
    double cuota=0;
    if ( query.isActive() ) {
          while ( query.next() )
              {
                bool esautofactura=query.value(10).toBool() || query.value(11).toBool()
                                   || query.value(16).toBool();
	        ui.progressBar->setValue(numprog);
                if (guardadoc!=query.value(0).toString() || query.value(0).toString().length()==0 
                    || numprog==1 || guardaprov!=query.value(2).toString()) numorden++; 
                guardadoc=query.value(0).toString();
                guardaprov=query.value(2).toString();
                if (ordenrecepcion)
                    global+=query.value(15).toString()+"\t";
                  else
                    global+=cadnum.setNum(numorden)+"\t";
	        global+=query.value(0).toString()+"\t"; // fra.
	        global+=query.value(9).toDate().toString("dd.MM.yyyy")+"\t"; // fecha fra
	        global+=query.value(1).toDate().toString("dd.MM.yyyy")+"\t"; // fecha contable
                if (!esautofactura)
                  {
                   global +=  ( query.value(14).toString().isEmpty() ?
                                cifcta(query.value(2).toString()) :
                                filtracad(query.value(14).toString()));
                   global +="\t";
                   // global+=query.value(2).toString()+"\t"; // cuenta
                   global+= (query.value(13).toString().isEmpty() ?
                             filtracad(query.value(3).toString()) :
                             filtracad(query.value(13).toString()))
                             +"\t"; // proveedor
	           //if (aib || eib) stream <<  nifcomunitariocta(query.value(2).toString());
		   //else stream <<  cifcta(query.value(2).toString()) // cif
                  }
                  else
                      {
                       global+=  basedatos::instancia()->cif(); // cif
                       global+="\t"; // cuenta
	               global+=filtracad(nombreempresa())+"\t"; // proveedor
                      }
                global+=query.value(4).toString()+"\t"; // asiento
                global+=formatea_redondeado_sep(query.value(5).toDouble(),comadecimal,decimales)+"\t"; // base iva
                global+=formatea_redondeado_sep(query.value(6).toDouble(),comadecimal,decimales)+"\t"; // tipo
                global+=formatea_redondeado_sep(query.value(7).toDouble(),comadecimal,decimales)+"\t"; // cuota
                global+=formatea_redondeado_sep(query.value(8).toDouble(),comadecimal,decimales)+"\t"; // total
                global+="\n";
                if (query.value(18).toBool())
                   {
                    // criterio de caja
                    QSqlQuery q=basedatos::instancia()->datos_venci_apunte_iva_caja_liquidados(
                            query.value(19).toString());
                    // v.fecha_vencimiento, v.liq_iva_fecha, v.forzar_liq_iva, v.importe,
                    // v.num, v.medio_realizacion, v.cuenta_bancaria
                    bool vacio=true;
                    if (q.isActive())
                        while (q.next())
                         {
                          vacio=false;
                          global+= "\t\t";
                          global+= tr("Importe pago");
                          global+= "\t";
                          global+= formatea_redondeado_sep(q.value(3).toDouble(),comadecimal,decimales);
                          global+= "\t";
                          global+= tr("FECHA:");
                          global+= "\t";
                          global+= q.value(1).toDate().toString("dd-MM-yyyy");
                          global+= "\t";
                          global+= q.value(5).toString();
                          global+= "\t\t";
                          global+= tr("Cuenta:");
                          global+= "\t";
                          global+= q.value(6).toString();
                          global+= "\n  ";
                         }
                    if (vacio)
                       {
                        global+= "\t\t";
                        global+= tr("Importe pago");
                        global+= "\t";
                        global+= "\t";
                        global+= tr("FECHA:");
                        global+= "\t";
                        global+= "\t";
                        global+= "\t\t";
                        global+= tr("Cuenta:");
                        global+= "\t";
                        global+= "\n  ";
                       }
                   }
                numprog++;
	        base+=query.value(5).toDouble();
	        cuota+=query.value(7).toDouble();
	      }
      }

   // --------------------------------------------------------------------------------------
    global+="\t\t\t\t\t\t"+tr("SUMAS TOTALES:")+"\t";
    global+=formatea_redondeado_sep(base,comadecimal,decimales) + "\t\t";
    global+=formatea_redondeado_sep(cuota,comadecimal,decimales) + "\t";
    global+=formatea_redondeado_sep(base+cuota,comadecimal,decimales);
    global+="\n";

   cb->setText(global);
   QMessageBox::information( this, tr("Libro de Facturas"),
                            tr("Se ha pasado el contenido al portapapeles") );
}


void libro_recibidas::copiahorizontal()
{
   QClipboard *cb = QApplication::clipboard();
   QString global;


   global=filtracad(nombreempresa()) + "\n" ;
   global+=ui.cabeceralineEdit->text();
   global+="\n";
   global+="\n";
   global+=(ordenrecepcion ? tr("Recep.") : tr("Orden"))+"\t";
   global+=tr("Factura")+"\t";
   global+=tr("Fecha Fra.")+"\t";
   global+=tr("Contab.")+"\t";
   global+=tr("Cuenta")+"\t";
   global+=tr("Proveedor/acreedor")+"\t";
   global+=tr("CIF/NIF")+"\t";
   global+=tr("Asto.")+"\t";
   global+=tr("Base Imp.")+"\t";
   global+=tr("Tipo")+"\t";
   global+=tr("Cuota")+"\t";
   global+=tr("Total")+"\t";
   global+="\n";

    QString ejercicio=ui.ejerciciocomboBox->currentText();
    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    int registros = basedatos::instancia()->num_registros_recibidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(),false, cuenta);

    QSqlQuery query =
      basedatos::instancia()->registros_recibidas(ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(), cuenta);

    ui.progressBar->setMaximum(registros);
    int numorden=0;
    int numprog=1;
    QString cadnum,guardadoc,guardaprov;
    double base=0;
    double cuota=0;
    if ( query.isActive() ) {
          while ( query.next() )
              {
                bool esautofactura=query.value(10).toBool() || query.value(11).toBool()
                                   || query.value(16).toBool();
	        ui.progressBar->setValue(numprog);
                if (guardadoc!=query.value(0).toString() || query.value(0).toString().length()==0 
                    || numprog==1 || guardaprov!=query.value(2).toString()) numorden++; 
                guardadoc=query.value(0).toString();
                guardaprov=query.value(2).toString();
                if (ordenrecepcion) global+=query.value(15).toString()+"\t";
                    else
                      global+=cadnum.setNum(numorden)+"\t";
	        global+=query.value(0).toString()+"\t"; // fra.
	        global+=query.value(9).toDate().toString("dd.MM.yyyy")+"\t"; // fecha fra
	        global+=query.value(1).toDate().toString("dd.MM.yyyy")+"\t"; // fecha contable
                if (!esautofactura)
                   {
	            global+=query.value(2).toString()+"\t"; // cuenta
                    global+= (query.value(13).toString().isEmpty() ?
                              filtracad(query.value(3).toString()) :
                              filtracad(query.value(13).toString()))
                              +"\t"; // proveedor
                    global+=(query.value(14).toString().isEmpty() ?
                             cifcta(query.value(2).toString()) :
                             query.value(14).toString()); // cif
                   }
                   else
                      {
	               global+="\t"; // cuenta
	               global+=filtracad(nombreempresa())+"\t"; // proveedor
                       global+=basedatos::instancia()->cif();
                      }

                global+="\t";
                global+=query.value(4).toString()+"\t"; // asiento
                global+=formatea_redondeado_sep(query.value(5).toDouble(),comadecimal,decimales)+"\t"; // base iva
                global+=formatea_redondeado_sep(query.value(6).toDouble(),comadecimal,decimales)+"\t"; // tipo
                global+=formatea_redondeado_sep(query.value(7).toDouble(),comadecimal,decimales)+"\t"; // cuota
                global+=formatea_redondeado_sep(query.value(8).toDouble(),comadecimal,decimales)+"\t"; // total
                global+="\n";
                if (query.value(18).toBool())
                   {
                    // criterio de caja
                    QSqlQuery q=basedatos::instancia()->datos_venci_apunte_iva_caja_liquidados(
                            query.value(19).toString());
                    // v.fecha_vencimiento, v.liq_iva_fecha, v.forzar_liq_iva, v.importe,
                    // v.num, v.medio_realizacion, v.cuenta_bancaria
                    bool vacio=true;
                    if (q.isActive())
                        while (q.next())
                         {
                          vacio=false;
                          global+= "\t\t";
                          global+= tr("Importe pago");
                          global+= "\t";
                          global+= formatea_redondeado_sep(q.value(3).toDouble(),comadecimal,decimales);
                          global+= "\t";
                          global+= tr("FECHA:");
                          global+= "\t";
                          global+= q.value(1).toDate().toString("dd-MM-yyyy");
                          global+= "\t";
                          global+= q.value(5).toString();
                          global+= "\t\t";
                          global+= tr("Cuenta:");
                          global+= "\t";
                          global+= q.value(6).toString();
                          global+= "\n  ";
                         }
                    if (vacio)
                       {
                        global+= "\t\t";
                        global+= tr("Importe pago");
                        global+= "\t";
                        global+= "\t";
                        global+= tr("FECHA:");
                        global+= "\t";
                        global+= "\t";
                        global+= "\t\t";
                        global+= tr("Cuenta:");
                        global+= "\t";
                        global+= "\n  ";
                       }
                   }
                numprog++;
	        base+=query.value(5).toDouble();
	        cuota+=query.value(7).toDouble();
	      }
      }

   // --------------------------------------------------------------------------------------
    global+="\t\t\t\t\t\t\t"+tr("SUMAS TOTALES:")+"\t";
    global+=formatea_redondeado_sep(base,comadecimal,decimales) + "\t\t";
    global+=formatea_redondeado_sep(cuota,comadecimal,decimales) + "\t";
    global+=formatea_redondeado_sep(base+cuota,comadecimal,decimales);
    global+="\n";

   cb->setText(global);
   QMessageBox::information( this, tr("Libro de Facturas"),
                            tr("Se ha pasado el contenido al portapapeles") );
}


bool libro_recibidas::generalatexprorrata()
{
   if (!ui.interiorescheckBox->isChecked() && !ui.aibcheckBox->isChecked() &&
       !ui.autofacturascheckBox->isChecked() && !ui.rectificativascheckBox->isChecked()
       && !ui.autofacturasnouecheckBox->isChecked()
       && !ui.agrariocheckBox->isChecked()
       && !ui.isp_op_interiorescheckBox->isChecked()
       && !ui.importacionescheckBox->isChecked()) return false;

   QString qfichero=dirtrabajo();
   qfichero.append(QDir::separator());
   qfichero+=nombrefichero();
   qfichero+=".tex";
   QFile fichero(adapta(qfichero));
   if (! fichero.open( QIODevice::WriteOnly ) ) return false;

    QTextStream stream( &fichero );
    stream.setCodec("UTF-8");

    stream << cabeceralatex();
    stream << margen_extra_latex();

    stream << "\\begin{landscape}" << "\n";

    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {";
    stream << filtracad(nombreempresa()) << "}}";
    stream << "\\end{center}";

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    if (!cuenta.isEmpty())
     {
      stream << "\\begin{center}" << "\n";
      stream << "{\\Large \\textbf {";
      stream << filtracad(cuenta) + " " + descripcioncuenta(cuenta) << "}}";
      stream << "\\end{center}";
     }

    stream << "\\begin{center}" << "\n";
    stream << "\\begin{longtable}{|r|p{2cm}|c|c|c|p{3.5cm}|l|r|r|r|r|r|r|r|r|}" << "\n";
    stream << "\\hline" << "\n";

   stream << "\\multicolumn{15}{|c|} {\\textbf{";
   QString cadena=filtracad(ui.cabeceralineEdit->text());
   // --------------------------------------------------------------------------------------
   stream << cadena;
   stream <<  "}} \\\\";
   stream << "\\hline" << "\n";
    // -------------------------------------------------------------------------------------
    if (ordenrecepcion) stream << "{\\tiny{" << tr("Recep.") << "}} & ";
     else
         stream << "{\\tiny{" << tr("Orden") << "}} & ";
    stream << "{\\tiny{" << tr("Factura") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha Fra.") << "}} & ";
    stream << "{\\tiny{" << tr("Contab.") << "}} & ";
    stream << "{\\tiny{" << tr("Cuenta") << "}} & ";
    stream << "{\\tiny{" << tr("Proveedor/acreedor") << "}} & ";
    stream << "{\\tiny{" << tr("CIF/NIF") << "}} & ";
    stream << "{\\tiny{" << tr("Asto.") << "}} & ";
    stream << "{\\tiny{" << tr("Base Imp.") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo") << "}} & ";
    stream << "{\\tiny{" << tr("Cuota") << "}} & ";
    stream << "{\\tiny{" << tr("Total") << "}} &";
    stream << "{\\tiny{" << tr("Prorrata") << "}} &";
    stream << "{\\tiny{" << tr("Afectación") << "}} &";
    stream << "{\\tiny{" << tr("Cuota ef.") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endfirsthead";
    // ----------------------------------------------------------------------------------------
    stream << "\\hline" << "\n";
    // stream << tr(" \\\\") << "\n";
    if (ordenrecepcion) stream << "{\\tiny{" << tr("Recep.") << "}} & ";
       else
          stream << "{\\tiny{" << tr("Orden") << "}} & ";
    stream << "{\\tiny{" << tr("Factura") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha Fra.") << "}} & ";
    stream << "{\\tiny{" << tr("Contab.") << "}} & ";
    stream << "{\\tiny{" << tr("Cuenta") << "}} & ";
    stream << "{\\tiny{" << tr("Proveedor/acreedor") << "}} & ";
    stream << "{\\tiny{" << tr("CIF/NIF") << "}} & ";
    stream << "{\\tiny{" << tr("Asto.") << "}} & ";
    stream << "{\\tiny{" << tr("Base Imp.") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo") << "}} & ";
    stream << "{\\tiny{" << tr("Cuota") << "}} & ";
    stream << "{\\tiny{" << tr("Total") << "}} &";
    stream << "{\\tiny{" << tr("Prorrata") << "}} &";
    stream << "{\\tiny{" << tr("Afectación") << "}} &";
    stream << "{\\tiny{" << tr("Cuota ef.") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endhead" << "\n";
    // ---------------------------------------------------------------------------------------


    QString ejercicio=ui.ejerciciocomboBox->currentText();
    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    int registros = basedatos::instancia()->num_registros_recibidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(),false, cuenta);

    QSqlQuery query =
      basedatos::instancia()->registros_recibidas_prorrata(ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(),false, cuenta);

    ui.progressBar->setMaximum(registros);
    int numorden=0;
    int numprog=1;
    QString cadnum,guardadoc,guardaprov;
    double base=0;
    double cuota=0;
    double cuotaefectiva=0;
    if ( query.isActive() ) {
          while ( query.next() )
              {
                bool esautofactura=query.value(12).toBool() || query.value(13).toBool()
                                   || query.value(19).toBool();
	        ui.progressBar->setValue(numprog);
                if (guardadoc!=query.value(0).toString() || query.value(0).toString().length()==0 
                    || numprog==1 || guardaprov!=query.value(2).toString()) numorden++; 
                guardadoc=query.value(0).toString();
                guardaprov=query.value(2).toString();
                stream << "{\\tiny " <<
                        (ordenrecepcion ? query.value(18).toString() : cadnum.setNum(numorden))
                        << "} & {\\tiny ";
	        stream << filtracad(query.value(0).toString()) << "} & {\\tiny "; // fra.
	        stream << query.value(11).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha fra
	        stream << query.value(1).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha contable
                if (!esautofactura)
                   {
	            stream <<  query.value(2).toString() << "} & {\\tiny "; // cuenta
                    stream << (query.value(15).toString().isEmpty() ?
                               filtracad(query.value(3).toString()) :
                               filtracad(query.value(15).toString()))
                               << "} & {\\tiny "; // proveedor
                    stream << (query.value(16).toString().isEmpty() ?
                               filtracad(cifcta(query.value(2).toString())) :
                               filtracad(query.value(16).toString()))
                           << "} & {\\tiny "; // cif
                   }
                   else
                    {
	             stream << "} & {\\tiny "; // cuenta
	             stream <<  filtracad(nombreempresa()) << "} & {\\tiny "; // proveedor
                     stream <<  basedatos::instancia()->cif() << "} & {\\tiny "; // cif
                    }
                stream <<  query.value(4).toString() << "} & {\\tiny "; // asiento
                stream <<  formatea_redondeado_sep(query.value(5).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // base iva
                stream <<  formatea_redondeado_sep(query.value(6).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // tipo
                stream <<  formatea_redondeado_sep(query.value(7).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // cuota
                stream <<  formatea_redondeado_sep(query.value(8).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // total
                stream <<  formatea_redondeado_sep(query.value(9).toDouble()*100,comadecimal,decimales)
                       << "} & {\\tiny "; // prorrata
                stream <<  formatea_redondeado_sep(query.value(14).toDouble()*100,comadecimal,decimales)
                       << "} & {\\tiny "; // afecto
                stream <<  formatea_redondeado_sep(query.value(10).toDouble(),comadecimal,decimales) << "} \\\\ \n  "
                       << "\\hline\n"; // cuota corregida prorrata
                if (query.value(21).toBool())
                   {
                    // criterio de caja
                    QSqlQuery q=basedatos::instancia()->datos_venci_apunte_iva_caja_liquidados(
                            query.value(17).toString());
                    // v.fecha_vencimiento, v.liq_iva_fecha, v.forzar_liq_iva, v.importe,
                    // v.num, v.medio_realizacion, v.cuenta_bancaria
                    bool vacio=true;
                    if (q.isActive())
                        while (q.next())
                         {
                          vacio=false;
                          stream << "\\multicolumn{3}{|r|} {\\tiny ";
                          stream << tr("Importe pago");
                          stream << "} & {\\tiny ";
                          stream << formatea_redondeado_sep(q.value(3).toDouble(),comadecimal,decimales);
                          stream << "} & \\multicolumn{2}{|l|} {\\tiny ";
                          stream << q.value(5).toString();
                          stream << "} & {\\tiny ";
                          stream << tr("FECHA:");
                          stream << "} & {\\tiny ";
                          stream << q.value(1).toDate().toString("dd-MM-yyyy");
                          stream << "} & {\\tiny ";
                          stream << tr("Cuenta:");
                          stream << "} & \\multicolumn{3}{|l|} {\\tiny ";
                          stream << q.value(6).toString();
                          stream << "} & \\multicolumn{3}{|l|} {\\tiny ";
                          stream << "} \\\\ \n  "
                                  << "\\hline\n";
                         }
                    if (vacio)
                       {
                        stream << "\\multicolumn{3}{|r|} {\\tiny ";
                        stream << tr("Importe pago");
                        stream << "} & {\\tiny ";
                        stream << "} & \\multicolumn{2}{|l|} {\\tiny ";
                        stream << "} & {\\tiny ";
                        stream << tr("FECHA:");
                        stream << "} & {\\tiny ";
                        stream << "} & {\\tiny ";
                        stream << tr("Cuenta:");
                        stream << "} & \\multicolumn{3}{|l|} {\\tiny ";
                        stream << "} & \\multicolumn{3}{|l|} {\\tiny ";
                        stream << "} \\\\ \n  "
                                << "\\hline\n";
                       }
                   }
                numprog++;
	        base+=query.value(5).toDouble();
	        cuota+=query.value(7).toDouble();
                cuotaefectiva+=query.value(10).toDouble();
	      }
      }

    stream << "\\multicolumn{8}{|r|} {\\textbf{";
   // --------------------------------------------------------------------------------------
    stream << tr("SUMAS TOTALES:") << "}} ";
    stream << " & {\\tiny " << formatea_redondeado_sep(base,comadecimal,decimales) << "} & & {\\tiny ";
    stream << formatea_redondeado_sep(cuota,comadecimal,decimales) << "} &  {\\tiny ";
    stream << formatea_redondeado_sep(base+cuota,comadecimal,decimales) << "} & & & {\\tiny ";
    stream << formatea_redondeado_sep(cuotaefectiva,comadecimal,decimales);
    stream <<  "} \\\\ \n";
    stream << "\\hline" << "\n";


    stream << "\\end{longtable}" << "\n";
    stream << "\\end{center}" << "\n";

    stream << "\\end{landscape}\n";

    stream << "% FIN_CUERPO\n";
    stream << "\\end{document}" << "\n";

    fichero.close();

    ui.progressBar->reset();

    return true;


}


void libro_recibidas::copiaprorrata()
{
   QClipboard *cb = QApplication::clipboard();
   QString global;

   global=filtracad(nombreempresa());
   global+="\n\n";
   QString cadena=ui.cabeceralineEdit->text();
   // --------------------------------------------------------------------------------------
   global+=cadena;
   global+="\n\n";
    // -------------------------------------------------------------------------------------
    if (ordenrecepcion) global+=tr("Orden")+"\t";
       else global+=tr("Recep.")+"\t";
    global+=tr("Factura")+"\t";
    global+=tr("Fecha Fra.")+"\t";
    global+=tr("Contab.")+"\t";
    global+=tr("Cuenta")+"\t";
    global+=tr("Proveedor/acreedor")+"\t";
    global+=tr("CIF/NIF")+"\t";
    global+=tr("Asto.")+"\t";
    global+=tr("Base Imp.")+"\t";
    global+=tr("Tipo")+"\t";
    global+=tr("Cuota")+"\t";
    global+=tr("Total")+"\t";
    global+=tr("Prorrata")+"\t";
    global+=tr("Afectación")+"\t";
    global+=tr("Cuota ef.")+"\n";

    QString ejercicio=ui.ejerciciocomboBox->currentText();
    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    int registros = basedatos::instancia()->num_registros_recibidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(),false, cuenta);

    QSqlQuery query =
      basedatos::instancia()->registros_recibidas_prorrata(ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(),false, cuenta);

    ui.progressBar->setMaximum(registros);
    int numorden=0;
    int numprog=1;
    QString cadnum,guardadoc,guardaprov;
    double base=0;
    double cuota=0;
    double cuotaefectiva=0;
    if ( query.isActive() ) {
          while ( query.next() )
              {
                bool esautofactura=query.value(12).toBool() || query.value(13).toBool()
                                   || query.value(19).toBool();
	        ui.progressBar->setValue(numprog);
                if (guardadoc!=query.value(0).toString() || query.value(0).toString().length()==0 
                    || numprog==1 || guardaprov!=query.value(2).toString()) numorden++; 
                guardadoc=query.value(0).toString();
                guardaprov=query.value(2).toString();
                global+=(ordenrecepcion ? query.value(18).toString() : cadnum.setNum(numorden))+ "\t";
	        global+=filtracad(query.value(0).toString()) + "\t"; // fra.
	        global+=query.value(11).toDate().toString("dd.MM.yyyy") + "\t"; // fecha fra
	        global+=query.value(1).toDate().toString("dd.MM.yyyy") + "\t"; // fecha contable
                if (!esautofactura)
                  {
	           global+=query.value(2).toString() + "\t"; // cuenta
                   global+= ( query.value(15).toString().isEmpty() ?
                              filtracad(query.value(3).toString()) :
                              filtracad(query.value(15).toString()) ) + "\t"; // proveedor
                   global+=( query.value(16).toString().isEmpty() ?
                             cifcta(query.value(2).toString()) :
                             filtracad(query.value(16).toString()) )+ "\t"; // cif
                  }
                  else
                     {
	              global+="\t"; // cuenta
	              global+=filtracad(nombreempresa()) + "\t"; // proveedor
                      global+= basedatos::instancia()->cif() + "\t"; // cif
                      }
                global+=query.value(4).toString() + "\t"; // asiento
                global+=formatea_redondeado_sep(query.value(5).toDouble(),comadecimal,decimales)+"\t"; // base iva
                global+=formatea_redondeado_sep(query.value(6).toDouble(),comadecimal,decimales)+"\t"; // tipo
                global+=formatea_redondeado_sep(query.value(7).toDouble(),comadecimal,decimales)+"\t"; // cuota
                global+=formatea_redondeado_sep(query.value(8).toDouble(),comadecimal,decimales)+"\t"; // total
                global+=formatea_redondeado_sep(query.value(9).toDouble(),comadecimal,decimales)+"\t"; // prorrata
                global+=formatea_redondeado_sep(query.value(14).toDouble(),comadecimal,decimales)+"\t"; // prorrata
                global+=formatea_redondeado_sep(query.value(10).toDouble(),comadecimal,decimales)+"\n"; // cuota corregida prorrata
                if (query.value(21).toBool())
                   {
                    // criterio de caja
                    QSqlQuery q=basedatos::instancia()->datos_venci_apunte_iva_caja_liquidados(
                            query.value(17).toString());
                    // v.fecha_vencimiento, v.liq_iva_fecha, v.forzar_liq_iva, v.importe,
                    // v.num, v.medio_realizacion, v.cuenta_bancaria
                    bool vacio=true;
                    if (q.isActive())
                        while (q.next())
                         {
                          vacio=false;
                          global+= "\t\t";
                          global+= tr("Importe pago");
                          global+= "\t";
                          global+= formatea_redondeado_sep(q.value(3).toDouble(),comadecimal,decimales);
                          global+= "\t";
                          global+= tr("FECHA:");
                          global+= "\t";
                          global+= q.value(1).toDate().toString("dd-MM-yyyy");
                          global+= "\t";
                          global+= q.value(5).toString();
                          global+= "\t\t";
                          global+= tr("Cuenta:");
                          global+= "\t";
                          global+= q.value(6).toString();
                          global+= "\n  ";
                         }
                    if (vacio)
                       {
                        global+= "\t\t";
                        global+= tr("Importe pago");
                        global+= "\t";
                        global+= "\t";
                        global+= tr("FECHA:");
                        global+= "\t";
                        global+= "\t";
                        global+= "\t\t";
                        global+= tr("Cuenta:");
                        global+= "\t";
                        global+= "\n  ";
                       }
                   }
                numprog++;
	        base+=query.value(5).toDouble();
	        cuota+=query.value(7).toDouble();
                cuotaefectiva+=query.value(10).toDouble();
	      }
      }

    global+= "\t\t\t\t\t\t\t";
   // --------------------------------------------------------------------------------------
    global+=tr("SUMAS TOTALES:") + "\t";
    global+=formatea_redondeado_sep(base,comadecimal,decimales) + "\t\t";
    global+=formatea_redondeado_sep(cuota,comadecimal,decimales) + "\t";
    global+=formatea_redondeado_sep(base+cuota,comadecimal,decimales) + "\t\t\t";
    global+=formatea_redondeado_sep(cuotaefectiva,comadecimal,decimales);
    global+="\n";


   cb->setText(global);
   QMessageBox::information( this, tr("Libro de Facturas"),
                            tr("Se ha pasado el contenido al portapapeles") );

}



bool libro_recibidas::generalatexsoportado_horizontal_bi()
{
   if (!ui.interiorescheckBox->isChecked() && !ui.aibcheckBox->isChecked() &&
       !ui.autofacturascheckBox->isChecked()  && !ui.rectificativascheckBox->isChecked() &&
       !ui.autofacturasnouecheckBox->isChecked() && !ui.agrariocheckBox->isChecked() &&
       !ui.isp_op_interiorescheckBox->isChecked() && !ui.importacionescheckBox->isChecked())
       return false;

   QString qfichero=dirtrabajo();
   qfichero.append(QDir::separator());
   qfichero+=nombrefichero();
   qfichero+=".tex";
   QFile fichero(adapta(qfichero));
   if (! fichero.open( QIODevice::WriteOnly ) ) return false;

    QTextStream stream( &fichero );
    stream.setCodec("UTF-8");
    stream << cabeceralatex();
    stream << margen_extra_latex();

    stream << "\\begin{landscape}" << "\n";

    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {";
    stream << filtracad(nombreempresa()) << "}}";
    stream << "\\end{center}";

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    if (!cuenta.isEmpty())
     {
      stream << "\\begin{center}" << "\n";
      stream << "{\\Large \\textbf {";
      stream << filtracad(cuenta) + " " + descripcioncuenta(cuenta) << "}}";
      stream << "\\end{center}";
     }

    stream << "\\begin{center}" << "\n";
    stream << "\\begin{longtable}{|r|p{2.5cm}|c|c|c|p{3.5cm}|l|p{3.5cm}|r|r|r|r|r|}" << "\n";
    stream << "\\hline" << "\n";

   stream << "\\multicolumn{13}{|c|} {\\textbf{";
   QString cadena=filtracad(ui.cabeceralineEdit->text());
   // --------------------------------------------------------------------------------------
   stream << cadena;
   stream <<  "}} \\\\";
   stream << "\\hline" << "\n";
    // -------------------------------------------------------------------------------------
   if (ordenrecepcion)
    stream << "{\\tiny{" << tr("Recep.") << "}} & ";
    else
        stream << "{\\tiny{" << tr("Orden") << "}} & ";
    stream << "{\\tiny{" << tr("Factura") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha Fra.") << "}} & ";
    stream << "{\\tiny{" << tr("Contab.") << "}} & ";
    stream << "{\\tiny{" << tr("Cuenta") << "}} & ";
    stream << "{\\tiny{" << tr("Proveedor/acreedor") << "}} & ";
    stream << "{\\tiny{" << tr("CIF/NIF") << "}} & ";
    stream << "{\\tiny{" << tr("Concepto") << "}} & ";
    stream << "{\\tiny{" << tr("Asto.") << "}} & ";
    stream << "{\\tiny{" << tr("Base Imp.") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo") << "}} & ";
    stream << "{\\tiny{" << tr("Cuota") << "}} & ";
    stream << "{\\tiny{" << tr("Total") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endfirsthead";
    // ----------------------------------------------------------------------------------------
    stream << "\\hline" << "\n";
    // stream << tr(" \\\\") << "\n";
    if (ordenrecepcion)
      stream << "{\\tiny{" << tr("Recep.") << "}} & ";
      else
         stream << "{\\tiny{" << tr("Orden") << "}} & ";
    stream << "{\\tiny{" << tr("Factura") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha Fra.") << "}} & ";
    stream << "{\\tiny{" << tr("Contab.") << "}} & ";
    stream << "{\\tiny{" << tr("Cuenta") << "}} & ";
    stream << "{\\tiny{" << tr("Proveedor/acreedor") << "}} & ";
    stream << "{\\tiny{" << tr("CIF/NIF") << "}} & ";
    stream << "{\\tiny{" << tr("Concepto") << "}} & ";
    stream << "{\\tiny{" << tr("Asto.") << "}} & ";
    stream << "{\\tiny{" << tr("Base Imp.") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo") << "}} & ";
    stream << "{\\tiny{" << tr("Cuota") << "}} & ";
    stream << "{\\tiny{" << tr("Total") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endhead" << "\n";
    // ---------------------------------------------------------------------------------------

    QString ejercicio=ui.ejerciciocomboBox->currentText();
    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    int registros = basedatos::instancia()->num_registros_recibidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(),false,cuenta);

    QSqlQuery query =
      basedatos::instancia()->registros_recibidas(ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(), cuenta);

    ui.progressBar->setMaximum(registros);
    int numorden=0;
    int numprog=1;
    QString cadnum,guardadoc,guardaprov;
    double base=0;
    double cuota=0;
    if ( query.isActive() ) {
          while ( query.next() )
              {
                bool esautofactura=query.value(10).toBool() || query.value(11).toBool()
                                   || query.value(16).toBool();
                ui.progressBar->setValue(numprog);
                if (guardadoc!=query.value(0).toString() || query.value(0).toString().length()==0
                    || numprog==1 || guardaprov!=query.value(2).toString()) numorden++;
                guardadoc=query.value(0).toString();
                guardaprov=query.value(2).toString();
                stream << "{\\tiny " <<
                        (ordenrecepcion ? query.value(15).toString():cadnum.setNum(numorden))
                        << "} & {\\tiny ";
                stream << filtracad(query.value(0).toString()) << "} & {\\tiny "; // fra.
                stream << query.value(9).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha fra
                stream << query.value(1).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha contable
                if (!esautofactura)
                   {
                    stream <<  query.value(2).toString() << "} & {\\tiny "; // cuenta
                    stream << (query.value(13).toString().isEmpty() ?
                               filtracad(query.value(3).toString()) :
                               filtracad(query.value(13).toString()))
                            << "} & {\\tiny "; // proveedor
                    stream << (query.value(14).toString().isEmpty() ?
                               filtracad(cifcta(query.value(2).toString())) :
                               filtracad(query.value(14).toString()))
                            << "} & {\\tiny "; // cif
                   }
                   else
                       {
                        stream <<  "} & {\\tiny "; // nada de cuenta
                        stream <<  filtracad(nombreempresa()) << "} & {\\tiny "; // proveedor
                        stream <<  basedatos::instancia()->cif() << "} & {\\tiny "; // cif
                       }

                stream << filtracad(descripcioncuenta(query.value(12).toString()))
                        << "} & {\\tiny "; // descripción cuenta gasto

                stream <<  query.value(4).toString() << "} & {\\tiny "; // asiento
                stream <<  formatea_redondeado_sep(query.value(5).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // base iva
                stream <<  formatea_redondeado_sep(query.value(6).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // tipo
                stream <<  formatea_redondeado_sep(query.value(7).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // cuota
                stream <<  formatea_redondeado_sep(query.value(8).toDouble(),comadecimal,decimales) << "} \\\\ \n  "
                       << "\\hline\n"; // total
                numprog++;
                base+=query.value(5).toDouble();
                cuota+=query.value(7).toDouble();
                if (query.value(18).toBool())
                   {
                    // criterio de caja
                    QSqlQuery q=basedatos::instancia()->datos_venci_apunte_iva_caja_liquidados(
                            query.value(19).toString());
                    // v.fecha_vencimiento, v.liq_iva_fecha, v.forzar_liq_iva, v.importe,
                    // v.num, v.medio_realizacion, v.cuenta_bancaria

                   }
              }
      }

    stream << "\\multicolumn{9}{|r|} {\\textbf{";
   // --------------------------------------------------------------------------------------
    stream << tr("SUMAS TOTALES:") << "}} ";
    stream << " & {\\tiny " << formatea_redondeado_sep(base,comadecimal,decimales) << "} & & {\\tiny ";
    stream << formatea_redondeado_sep(cuota,comadecimal,decimales) << "} &  {\\tiny ";
    stream << formatea_redondeado_sep(base+cuota,comadecimal,decimales);
    stream <<  "} \\\\ \n";
    stream << "\\hline" << "\n";


    stream << "\\end{longtable}" << "\n";
    stream << "\\end{center}" << "\n";

    stream << "\\end{landscape}\n";

    stream << "% FIN_CUERPO\n";
    stream << "\\end{document}" << "\n";

    fichero.close();

    ui.progressBar->reset();

    return true;

}


void libro_recibidas::copiahorizontal_bi()
{
   QClipboard *cb = QApplication::clipboard();
   QString global;


   global=filtracad(nombreempresa()) + "\n" ;
   global+=ui.cabeceralineEdit->text();
   global+="\n";
   global+="\n";
   global+=(ordenrecepcion ? tr("Recep.") : tr("Orden"))+"\t";
   global+=tr("Factura")+"\t";
   global+=tr("Fecha Fra.")+"\t";
   global+=tr("Contab.")+"\t";
   global+=tr("Cuenta")+"\t";
   global+=tr("Proveedor/acreedor")+"\t";
   global+=tr("CIF/NIF")+"\t";
   global+=tr("Concepto")+"\t";
   global+=tr("Asto.")+"\t";
   global+=tr("Base Imp.")+"\t";
   global+=tr("Tipo")+"\t";
   global+=tr("Cuota")+"\t";
   global+=tr("Total")+"\t";
   global+="\n";

    QString ejercicio=ui.ejerciciocomboBox->currentText();
    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    int registros = basedatos::instancia()->num_registros_recibidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(),false, cuenta);

    QSqlQuery query =
      basedatos::instancia()->registros_recibidas(ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.binversiongroupBox->isChecked(),
         ui.solobiradioButton->isChecked(),
         ui.sinbiradioButton->isChecked(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.agrariocheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         ui.importacionescheckBox->isChecked(), cuenta);

    ui.progressBar->setMaximum(registros);
    int numorden=0;
    int numprog=1;
    QString cadnum,guardadoc,guardaprov;
    double base=0;
    double cuota=0;
    if ( query.isActive() ) {
          while ( query.next() )
              {
                bool esautofactura=query.value(10).toBool() || query.value(11).toBool()
                                   || query.value(16).toBool();
                ui.progressBar->setValue(numprog);
                if (guardadoc!=query.value(0).toString() || query.value(0).toString().length()==0
                    || numprog==1 || guardaprov!=query.value(2).toString()) numorden++;
                guardadoc=query.value(0).toString();
                guardaprov=query.value(2).toString();
                global+=(ordenrecepcion ? query.value(15).toString() : cadnum.setNum(numorden))+"\t";
                global+=query.value(0).toString()+"\t"; // fra.
                global+=query.value(9).toDate().toString("dd.MM.yyyy")+"\t"; // fecha fra
                global+=query.value(1).toDate().toString("dd.MM.yyyy")+"\t"; // fecha contable
                if (!esautofactura)
                   {
                    global+=query.value(2).toString()+"\t"; // cuenta
                    global+= (query.value(13).toString().isEmpty() ?
                              filtracad(query.value(3).toString()) :
                              filtracad(query.value(13).toString()))
                              +"\t"; // proveedor
                    global+= (query.value(14).toString().isEmpty() ?
                              cifcta(query.value(2).toString()) :
                              filtracad(query.value(14).toString())); // cif
                   }
                   else
                      {
                       global+="\t"; // cuenta
                       global+=filtracad(nombreempresa())+"\t"; // proveedor
                       global+=basedatos::instancia()->cif();
                      }

                global+="\t";
                global+=filtracad(descripcioncuenta(query.value(12).toString()));
                global+="\t";
                global+=query.value(4).toString()+"\t"; // asiento
                global+=formatea_redondeado_sep(query.value(5).toDouble(),comadecimal,decimales)+"\t"; // base iva
                global+=formatea_redondeado_sep(query.value(6).toDouble(),comadecimal,decimales)+"\t"; // tipo
                global+=formatea_redondeado_sep(query.value(7).toDouble(),comadecimal,decimales)+"\t"; // cuota
                global+=formatea_redondeado_sep(query.value(8).toDouble(),comadecimal,decimales)+"\t"; // total
                global+="\n";
                numprog++;
                base+=query.value(5).toDouble();
                cuota+=query.value(7).toDouble();
              }
      }

   // --------------------------------------------------------------------------------------
    global+="\t\t\t\t\t\t\t\t"+tr("SUMAS TOTALES:")+"\t";
    global+=formatea_redondeado_sep(base,comadecimal,decimales) + "\t\t";
    global+=formatea_redondeado_sep(cuota,comadecimal,decimales) + "\t";
    global+=formatea_redondeado_sep(base+cuota,comadecimal,decimales);
    global+="\n";

   cb->setText(global);
   QMessageBox::information( this, tr("Libro de Facturas"),
                            tr("Se ha pasado el contenido al portapapeles") );
}



void libro_recibidas::latex()
{
    if (ui.prorratacheckBox->isChecked())
       {
        if (!generalatexprorrata())
           return;
       }
      else
        {
         if (!ui.horizontalradioButton->isChecked())
           {
            if  (!generalatexsoportado())
             return;
           }
         if (ui.horizontalradioButton->isChecked())
          {
           if  (!generalatexsoportado_horizontal())
           return;
          }
        }

    int valor=editalatex(nombrefichero());
    if (valor==1)
        QMessageBox::warning( this, tr("LIBRO FACTURAS"),tr("PROBLEMAS al llamar al editor Latex"));

}


void libro_recibidas::tabla()
{
    QString ejercicio=ui.ejerciciocomboBox->currentText();

    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    tabla_iva_sop *t = new tabla_iva_sop(usuario, comadecimal, decimales);
    t->pasa_info(ejercicio,
                 fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
                 ui.interiorescheckBox->isChecked(),
                 ui.aibcheckBox->isChecked(),
                 ui.autofacturascheckBox->isChecked(),
                 ui.autofacturasnouecheckBox->isChecked(),
                 ui.rectificativascheckBox->isChecked(),
                 ui.seriesgroupBox->isChecked(),
                 ui.serieiniciallineEdit->text(),
                 ui.seriefinallineEdit->text(),
                 ui.binversiongroupBox->isChecked(),
                 ui.solobiradioButton->isChecked(),
                 ui.sinbiradioButton->isChecked(),
                 ui.sujeciongroupBox->isChecked(),
                 ui.solonosujetasradioButton->isChecked(),
                 ui.excluirnosujetasradioButton->isChecked(),
                 ui.agrariocheckBox->isChecked(),
                 ui.isp_op_interiorescheckBox->isChecked(),
                 ui.importacionescheckBox->isChecked(),cuenta);
    t->exec();
    delete(t);

}


void libro_recibidas::cuenta_finedicion()
{
    ui.cuentalineEdit->setText(expandepunto(ui.cuentalineEdit->text(),anchocuentas()));
}


void libro_recibidas::boton_cuenta_pulsado()
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


void libro_recibidas::cuenta_cambiada()
{
 QString cadena;
 if (existecodigoplan(ui.cuentalineEdit->text(),&cadena))
    ui.descripcuentalabel->setText(cadena);
  else
    ui.descripcuentalabel->setText("");
}
