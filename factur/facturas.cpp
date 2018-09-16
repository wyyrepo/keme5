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

#include "facturas.h"
#include <QSqlQuery>
#include "funciones.h"
#include "factura.h"
#include "basedatos.h"
#include "traspasos.h"
#include "editarasiento.h"
#include <QMessageBox>
#include <QProgressDialog>

FacSqlModel::FacSqlModel(QObject *parent)
        : QSqlQueryModel(parent)
    {
     // primerasiento1=numeracionrelativa();
     comadecimal=haycomadecimal(); decimales=haydecimales();
    }


QVariant FacSqlModel::datagen(const QModelIndex &index, int role) const
    {
        QVariant value = QSqlQueryModel::data(index, role);
        return value;
    }

QVariant FacSqlModel::data(const QModelIndex &index, int role) const
    {
        QVariant value = QSqlQueryModel::data(index, role);
        if (value.isValid() && role == Qt::DisplayRole) {
            if (index.column() == 1)
               {
                // número de factura
               }
            if (index.column() == 4)
               {
                // fecha
                return value.toDate().toString(Qt::SystemLocaleDate);
               }
            if (index.column() == 5 || index.column() == 6)
               {
                    if (value.toBool())
                        return QString("*");
                     else return QString();
               }
            if (index.column() == 7)
               {
                // número de apunte en diario
                if (value.toInt()==0) return QString();
               }
        }
        if (role == Qt::TextAlignmentRole &&
             (index.column() == 1 || index.column() == 7))
            return qVariantFromValue(int(Qt::AlignVCenter | Qt::AlignRight));
        if (role == Qt::TextAlignmentRole && index.column()>=4 && index.column()<=6)
            return qVariantFromValue(int(Qt::AlignCenter));
        return value;
    }



facturas::facturas(QString qusuario) : QDialog() {
    ui.setupUi(this);

usuario=qusuario;
ui.facstableView->setSelectionBehavior(QAbstractItemView::SelectRows);
ui.facstableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

ui.serieinicialcomboBox->addItems(basedatos::instancia()->listacodseries());
ui.seriefinalcomboBox->addItems(basedatos::instancia()->listacodseries());

ui.seriefinalcomboBox->setCurrentIndex(ui.seriefinalcomboBox->count()-1);

QDate finicial=basedatos::instancia()->selectMinaerturaejerciciosnocerrado();
QDate ffinal=basedatos::instancia()->selectMaxcierrecountcierreejercicios();

ui.inicialdateEdit->setDate(finicial);
ui.finaldateEdit->setDate(ffinal);

connect(ui.nuevapushButton, SIGNAL(clicked()), this,
          SLOT (nuevafactura()));

connect(ui.editarpushButton, SIGNAL(clicked()), this,
          SLOT (editafactura()));

connect(ui.imprimirpushButton, SIGNAL(clicked()), this,
          SLOT (latexdoc()));

connect(ui.transferirpushButton, SIGNAL(clicked()), this,
          SLOT (exectrasp()));

connect(ui.contabilizarpushButton, SIGNAL(clicked()), this,
          SLOT (contabilizar()));

connect(ui.conttodopushButton,SIGNAL(clicked()),this,SLOT(contabilizartodas()));

connect(ui.borrarpushButton, SIGNAL(clicked()), this, SLOT(borralinea()));

connect(ui.verasientopushButton, SIGNAL(clicked()), this, SLOT(consultaasiento()));

connect(ui.numiniciallineEdit, SIGNAL(textChanged(QString)), this, SLOT (numini_cambiado()));

connect(ui.cuentainilineEdit, SIGNAL(textChanged(QString)), this, SLOT (cuentaini_cambiada()));

connect(ui.refrescarpushButton, SIGNAL(clicked()), this, SLOT(boton_refrescar()));

connect(ui.xmlpushButton , SIGNAL(clicked()), this,
          SLOT (xmldoc()));

connect(ui.informepushButton , SIGNAL(clicked()), this,
          SLOT (consultainforme()));

connect(ui.consultapushButton , SIGNAL(clicked()), this,
          SLOT (consulta()));

connect(ui.borra_asientopushButton, SIGNAL(clicked()), this,
          SLOT (borraasiento()));

connect(ui.edtexpushButton , SIGNAL(clicked()), this,
          SLOT (edita_tex()));

}

void facturas::nuevafactura()
{
    factura *f = new factura();
    f->exec();
    delete(f);
    refresca();
}


void facturas::activaconfiltro()
{
        FacSqlModel *elmodelo = new FacSqlModel;
        actufiltro();
        model = elmodelo;

        model->setQuery( basedatos::instancia()->select_cab_facturas (guardafiltro) );
        // serie,numero,cuenta, fecha_fac,
        // contabilizado,contabilizable, con_ret,
        // pase_diario_cta
        model->setHeaderData(0, Qt::Horizontal, tr("serie"));
        model->setHeaderData(1, Qt::Horizontal, tr("numero"));
        model->setHeaderData(2, Qt::Horizontal, tr("cuenta"));
        model->setHeaderData(3, Qt::Horizontal, tr("descripción"));

        model->setHeaderData(4, Qt::Horizontal, tr("fecha fact."));
        model->setHeaderData(5, Qt::Horizontal, tr("Contabilizado"));
        model->setHeaderData(6, Qt::Horizontal, tr("Cerrado"));
        model->setHeaderData(7, Qt::Horizontal, tr("Apunte diario"));
        model->setHeaderData(8, Qt::Horizontal, tr("TIPO DOC."));

        ui.facstableView->setModel(model);
        ui.facstableView->setAlternatingRowColors ( true);

        ui.facstableView->setColumnWidth(0,60);
        ui.facstableView->setColumnWidth(3,130);
        ui.facstableView->setColumnWidth(6,80);

}


void facturas::pasafiltro(QString filtro)
{
 model->setQuery( basedatos::instancia()->select_cab_facturas (filtro) );

}

void facturas::refresca()
{
 actufiltro();
 model->clear();
 model->setQuery(basedatos::instancia()->select_cab_facturas (guardafiltro));
 model->setHeaderData(0, Qt::Horizontal, tr("serie"));
 model->setHeaderData(1, Qt::Horizontal, tr("numero"));
 model->setHeaderData(2, Qt::Horizontal, tr("cuenta"));
 model->setHeaderData(3, Qt::Horizontal, tr("descripción"));
 model->setHeaderData(4, Qt::Horizontal, tr("fecha fact."));
 model->setHeaderData(5, Qt::Horizontal, tr("Contabilizado"));
 model->setHeaderData(6, Qt::Horizontal, tr("Cerrado"));
 model->setHeaderData(7, Qt::Horizontal, tr("Apunte diario"));
 model->setHeaderData(8, Qt::Horizontal, tr("TIPO DOC."));

}


void facturas::boton_refrescar()
{
 refresca();
}

void facturas::editafactura()
{
    if (basedatos::instancia()->doc_cerrado(serie(),
                            numero()))
      {
          QMessageBox::warning( this, tr("Contabilizar"),tr("ERROR: "
                               "El documento seleccionado está cerrado"));
          return;

      }

    if (!ui.facstableView->currentIndex().isValid())
    {
     QMessageBox::warning( this, tr("Edición de documentos"),
                           tr("Para editar un documento hay que seleccionarlo de la lista"));
     return;
    }
    QSqlQuery q;
    q = basedatos::instancia()->select_cabecera_doc (serie(), numero());
    if (q.isActive())
       if (q.next())
        {
         factura *f = new factura();
         // "select serie, numero, cuenta, fecha, fecha_fac, fecha_op, "
         // "contabilizado,contabilizable, con_ret, re, tipo_ret, retencion, "
         // "tipo_doc, notas, pie1, pie2, pase_diario_cta "
         f->pasa_cabecera_doc(q.value(0).toString(),
                         q.value(1).toString(),
                         q.value(2).toString(),
                         q.value(4).toDate(),
                         q.value(5).toDate(),
                         q.value(7).toBool(),
                         q.value(8).toBool(),
                         q.value(9).toBool(),
                         q.value(10).toString(),
                         q.value(11).toString(),
                         q.value(12).toString(),
                         q.value(13).toString(),
                         q.value(14).toString(),
                         q.value(15).toString());
         f->exec();
         delete(f);
         refresca();
       }
}


QString facturas::serie()
{
 if (!ui.facstableView->currentIndex().isValid()) return "";
 int fila=ui.facstableView->currentIndex().row();
 return model->data(model->index(fila,0),Qt::DisplayRole).toString();
}


QString facturas::numero()
{
 if (!ui.facstableView->currentIndex().isValid()) return "";
 int fila=ui.facstableView->currentIndex().row();
 return model->data(model->index(fila,1),Qt::DisplayRole).toString();
}


void facturas::xmldoc()
{
    if (!ui.facstableView->currentIndex().isValid())
      {
       QMessageBox::warning( this, tr("Generar XML"),
                           tr("Para generar un documento hay que seleccionarlo de la lista"));
       return;
      }

    fxmldoc(serie(),numero());

     QProgressDialog progreso(tr("Procesando información ..."), 0, 0, 3);
     progreso.setMinimumDuration(100);
     progreso.setWindowModality(Qt::WindowModal);
     progreso.setValue(1);
     QCoreApplication::processEvents();

    // QMessageBox::information( this, tr("Generar XML"),
    //                    tr("El fichero XML se ha generado"));

    QString cadfichjasper=trayreport();
    cadfichjasper.append(QDir::separator());

    // ------------------------------------
    QSqlQuery q;
    q = basedatos::instancia()->select_cabecera_doc (serie(), numero());
    QString tipo_doc;
    bool conret=false;
    if (q.isActive())
    {
       if (q.next())
        {
         // "select serie, numero, cuenta, fecha, fecha_fac, fecha_op, "
         // "contabilizado,contabilizable, con_ret, con_re, tipo_ret, retencion, "
         // "tipo_doc, notas, pie1, pie2, pase_diario_cta, clave "
        tipo_doc=q.value(12).toString();
        conret=q.value(8).toBool();
        } else return;
    }
    if (conret) cadfichjasper+="factura_keme_retencion.jasper";
       else cadfichjasper+="factura_keme.jasper";
    // -------------------------------------
    QString qfichero=dirtrabajodocs(tipo_doc); // el directorio será dirtrabajo() + ruta tipo doc
    qfichero.append(QDir::separator());
    QString rutagraf=qfichero+tipo_doc+".png";

    // generamos imagen del logo
    QString imagen=basedatos::instancia()->imagendoc(tipo_doc);
    if (imagen.isEmpty()) imagen=logo_facturacion_defecto();
    QPixmap foto;
    if (imagen.length()>0)
          {
           QByteArray byteshexa;
           byteshexa.append ( imagen );
           QByteArray bytes;
           bytes=bytes.fromHex ( byteshexa );
           foto.loadFromData ( bytes, "PNG");
           foto.save(rutagraf,"PNG");
          }

    // -------------------------------------------
    QString cadfich=serie()+numero()+".xml";
    QString pasa=qfichero+cadfich;

    QString cadpdf=qfichero+serie()+numero()+".pdf";

    progreso.setValue(2);
    QCoreApplication::processEvents();

    informe_jasper_xml(cadfichjasper, pasa,
                       "/Documento/Detalle/Linea", cadpdf,
                       rutagraf);

    progreso.setValue(3);
    QCoreApplication::processEvents();
    // QMessageBox::information( this, tr("XML DIARIO"),tr("El archivo XML se ha generado"));
}


void facturas::latexdoc()
{
    if (!ui.facstableView->currentIndex().isValid())
      {
       QMessageBox::warning( this, tr("Impresión de documentos"),
                           tr("Para generar un documento hay que seleccionarlo de la lista"));
       return;
      }

    QString fichero=latex_doc(serie(),numero());
    if (fichero.isEmpty())
    {
     QMessageBox::warning( this, tr("Edición de documentos"),
                           tr("ERROR: No se ha podido generar Latex"));
     return;
    }

    if (!genera_pdf_latex(fichero))
       {
        QMessageBox::information( this, tr("FACTURA EN PDF"),
                                  tr("ERROR: no ha sido posible generar el archivo PDF"));
        return;
       }
    QString archivopdf=fichero;
    archivopdf.truncate(archivopdf.length()-4);
    archivopdf.append(".pdf");
    if (!ejecuta(aplicacionabrirfich(extensionfich(archivopdf)),archivopdf))
          QMessageBox::warning( this, tr("FACTURA EN PDF"),
                              tr("No se puede abrir ")+archivopdf+tr(" con ")+
                              aplicacionabrirfich(extensionfich(archivopdf)) + "\n" +
                              tr("Verifique el diálogo de preferencias"));

}

void facturas::edita_tex()
{
    if (!ui.facstableView->currentIndex().isValid())
      {
       QMessageBox::warning( this, tr("Editar latex documento"),
                           tr("Para editar un documento hay que seleccionarlo de la lista"));
       return;
      }

    QString fichero=latex_doc(serie(),numero());
    if (fichero.isEmpty())
    {
     QMessageBox::warning( this, tr("Edición latex documento"),
                           tr("ERROR: No se ha podido generar Latex"));
     return;
    }

    int valor=editalatex_abs(fichero);

  if (valor==1)
    QMessageBox::warning( this, tr("Consulta de mayor"),
                          tr("PROBLEMAS al llamar al editor Latex"));
}


void facturas::exectrasp()
{
    QItemSelectionModel *seleccion;
    seleccion=ui.facstableView->selectionModel();
    QStringList lserie,lnumero;
    if (seleccion->hasSelection())
       {
         QList<QModelIndex> listaserie;
         QList<QModelIndex> listanumero;
         listaserie= seleccion->selectedRows ( 0 );
         listanumero=seleccion->selectedRows (1);


         if (listaserie.size()<1)
          {
             QMessageBox::warning( this, tr("Traspasos"),tr("ERROR: "
                                "se debe de seleccionar al menos un documento"));
           return;
          }

         // comprobamos que no haya ningún documento ya cerrado
         for (int i = 0; i < listaserie.size(); ++i)
            {
             if (basedatos::instancia()->doc_cerrado(model->datagen(listaserie.at(i),Qt::DisplayRole).toString(),
                                                     model->datagen(listanumero.at(i),Qt::DisplayRole).toString()))
             {
               QMessageBox::warning( this, tr("Traspasos"),tr("ERROR: "
                                    "alguno/s de los documentos seleccionados está cerrado"));
               return;

             }
            }

         // comprobamos que todos los clientes sean los mismos
         QString cliente=(basedatos::instancia()->doc_cliente(model->datagen(listaserie.at(0),Qt::DisplayRole).toString(),
                                                              model->datagen(listanumero.at(0),Qt::DisplayRole).toString()));
         for (int i = 1; i < listaserie.size(); ++i)
            {
             if (basedatos::instancia()->doc_cliente(model->datagen(listaserie.at(i),Qt::DisplayRole).toString(),
                                                     model->datagen(listanumero.at(i),Qt::DisplayRole).toString()) != cliente)
             {
               QMessageBox::warning( this, tr("Traspasos"),tr("ERROR: "
                                    "Los documentos no pertenecen a la misma cuenta cliente"));
               return;

             }
            }

         // cuenta, fecha, fecha operación, retención, recargo de equivalencia
         // mejor las cogemos del último documento

         // pasamos contenidos a listas para la plantilla
         for (int i = 0; i < listaserie.size(); ++i)
              {
               lserie << model->datagen(listaserie.at(i),Qt::DisplayRole).toString();
               lnumero << model->datagen(listanumero.at(i),Qt::DisplayRole).toString();
              }
       }


    traspasos *t = new traspasos(lserie,lnumero);
    t->exec();
    delete(t);
    refresca();

}

void facturas::contabilizar()
{
    // es contabilizable ??
    if (!basedatos::instancia()->doc_contabilizable(serie(),
                            numero()))
      {
          QMessageBox::warning( this, tr("Contabilizar"),tr("ERROR: "
                               "El documento seleccionado no es contabilizable"));
          return;

      }

    // está ya contabilizada ??
    if (basedatos::instancia()->doc_contabilizado(serie(),numero()))
    {
        QMessageBox::warning( this, tr("Contabilizar"),tr("ERROR: "
                             "El documento seleccionado ya está contabilizado"));
        return;

    }
    // realizar función aparte
    contabilizar_factura(serie(), numero(), usuario, false);
    refresca();
}

int facturas::numero_de_filas()
{
   return model->rowCount();
}


void facturas::contabilizartodas()
{
  int filas = numero_de_filas();
  for (int veces=filas-1; veces>=0; veces--)
     {
      QString cadnum; cadnum.setNum(veces);
      // QMessageBox::warning( this, tr("Contabilizar"),cadnum);
      QString qserie=model->data(model->index(veces,0),Qt::DisplayRole).toString();
      QString qnumero=model->data(model->index(veces,1),Qt::DisplayRole).toString();
      if (!basedatos::instancia()->doc_contabilizable(qserie,
                              qnumero)) continue;
      if (basedatos::instancia()->doc_contabilizado(qserie,qnumero)) continue;
      contabilizar_factura(qserie, qnumero, usuario, true);
     }
  refresca();
}

void facturas::borralinea()
{
    // está contabilizada ??
    if (basedatos::instancia()->doc_contabilizado(serie(),numero()))
    {
        QMessageBox::warning( this, tr("Borrar registro"),tr("ERROR: "
                             "El documento seleccionado está contabilizado"));
        return;

    }

    if (QMessageBox::question(
             this,
             tr("Borrar registro"),
             tr("¿ Desea borrar el documento seleccionado ?"),
             tr("&Sí"), tr("&No"),
             QString::null, 0, 1 ) ==1 )
                           return;


    // borrar
    int clave=basedatos::instancia()->clave_doc_fac(serie(),numero());
    // borramos registros de detalle
    basedatos::instancia()->borralineas_doc(clave);
    // borramos cabecera
    // contabilizar_factura(serie(), numero(), usuario);
    basedatos::instancia()->borra_cabecera_doc(serie(),numero());
    QMessageBox::information( this, tr("Borrar registro"),tr("Se ha eliminado el registro seleccionado"));

    refresca();

}


void facturas::consultaasiento()
{
 if (apunte_diario()=="")
       {
         QMessageBox::warning( this, tr("CONSULTAR ASIENTO"),
         tr("Para consultar asiento debe de seleccionar un registro con apunte en el diario"));
         return;
       }
QString elasiento=basedatos::instancia()->selectAsientodiariopase(apunte_diario());
QString ejercicio=ejerciciodelafecha(basedatos::instancia()->select_fecha_diariopase(apunte_diario()));
consultarasiento(elasiento,usuario,ejercicio);
}


QString facturas::apunte_diario()
{
 if (!ui.facstableView->currentIndex().isValid()) return "";
 int fila=ui.facstableView->currentIndex().row();
 return model->data(model->index(fila,7),Qt::DisplayRole).toString();
}

void facturas::numini_cambiado()
{
  ui.numfinallineEdit->setText(ui.numiniciallineEdit->text());
}

void facturas::cuentaini_cambiada()
{
 ui.cuentafinlineEdit->setText(ui.cuentainilineEdit->text());
}

void facturas::actufiltro()
{
 // la variable es guardafiltro
 guardafiltro=" where fecha_fac>='";
 guardafiltro+=ui.inicialdateEdit->date().toString("yyyy-MM-dd");
 guardafiltro+="' and fecha_fac<='";
 guardafiltro+=ui.finaldateEdit->date().toString("yyyy-MM-dd");
 guardafiltro+="' and serie>='";
 guardafiltro+=ui.serieinicialcomboBox->currentText();
 guardafiltro+="' and serie<='";
 guardafiltro+=ui.seriefinalcomboBox->currentText();
 guardafiltro+="'";
 if (!ui.numfinallineEdit->text().isEmpty() || !ui.numiniciallineEdit->text().isEmpty())
   {
    guardafiltro+=" and numero>=";
    guardafiltro+=ui.numiniciallineEdit->text();
    guardafiltro+=" and numero<=";
    guardafiltro+=ui.numfinallineEdit->text();
   }
 if (!ui.cuentafinlineEdit->text().isEmpty() || !ui.cuentainilineEdit->text().isEmpty())
   {
    guardafiltro+=" and cuenta>='";
    guardafiltro+=ui.cuentainilineEdit->text();
    guardafiltro+="' and cuenta<='";
    guardafiltro+=ui.cuentafinlineEdit->text();
    guardafiltro+="'";
   }
 if (ui.nocontaradioButton->isChecked())
     guardafiltro+=" and not contabilizado";
 if (ui.contaradioButton->isChecked())
     guardafiltro+=" and contabilizado";
 // guardafiltro+=" order by clave desc";
}


void facturas::latexinforme()
{
    bool coma=haycomadecimal();
    bool decimales=haydecimales();
    refresca();

    // vamos a imprimir condiciones del filtro
    // por línea: serie, numero, cuenta, descripcuenta, fecha factura, importe
    QString qfichero=dirtrabajo();
    qfichero.append(QDir::separator());
    qfichero=qfichero+tr("documentos.tex");
    QFile fichero(adapta(qfichero));

    if (! fichero.open( QIODevice::WriteOnly ) ) return;
    QTextStream stream( &fichero );
    stream.setCodec("UTF-8");
    stream << cabeceralatex();
    stream << margen_extra_latex();

    stream << "\\begin{center}\n"
              "{\\Large \\textbf {";
    stream << filtracad(nombreempresa());
    stream << "}}\n";
    stream << "\\end{center}\n\n";

    stream << "\\begin{flushleft}\n";
    stream << "{\\tiny{";
    stream << filtracad(tr("FILTRO: ")) + filtracad(guardafiltro);
    stream << "}}";
    stream << "\\end{flushleft}\n\n";


    stream << "\\begin{center}" << "\n";
    stream << "\\begin{longtable}{|l|l|c|p{5cm}|c|r|}" << "\n";
    stream << "\\hline" << "\n";

   stream << "\\multicolumn{6}{|c|} {\\textbf{";
   QString cadena;
   cadena=tr("LISTADO DE DOCUMENTOS");

   // --------------------------------------------------------------------------------------
   stream << cadena;
   stream <<  "}} \\\\";
    stream << "\\hline" << "\n";
    // -------------------------------------------------------------------------------------------------------
    stream << "{\\tiny{" << tr("Serie") << "}} & ";
    stream << "{\\tiny{" << tr("Número") << "}} & ";
    stream << "{\\tiny{" << tr("Cuenta") << "}} & ";
    stream << "{\\tiny{" << tr("Descripción cuenta") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha") << "}} & ";
    stream << "{\\tiny{" << tr("Importe") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endfirsthead";
    // --------------------------------------------------------------------------------------------------------
    stream << "\\hline" << "\n";
   // stream << tr(" \\\\") << "\n";
   stream << "\\multicolumn{6}{|c|} {\\textbf{";
   cadena=tr("LISTADO DE DOCUMENTOS");

   // --------------------------------------------------------------------------------------
   stream << cadena;
   stream <<  "}} \\\\";
    stream << "\\hline" << "\n";
    stream << "{\\tiny{" << tr("Serie") << "}} & ";
    stream << "{\\tiny{" << tr("Número") << "}} & ";
    stream << "{\\tiny{" << tr("Cuenta") << "}} & ";
    stream << "{\\tiny{" << tr("Descripción cuenta") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha") << "}} & ";
    stream << "{\\tiny{" << tr("Importe") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endhead" << "\n";
    // ---------------------------------------------------------------------------------------------------------
    int veces=0;
          while ( veces<model->rowCount())
               {
                double importe_factura=total_documento(
                         model->data(model->index(veces,0),Qt::DisplayRole).toString(),
                         model->data(model->index(veces,1),Qt::DisplayRole).toString());
                stream << "{\\tiny " <<
                        filtracad(model->data(model->index(veces,0),Qt::DisplayRole).toString())
                     << "} & {\\tiny ";
                stream << filtracad(model->data(model->index(veces,1),Qt::DisplayRole).toString())
                        << "} & {\\tiny ";
                stream << filtracad(model->data(model->index(veces,2),Qt::DisplayRole).toString())
                        << "} & {\\tiny ";
                stream <<
                 descripcioncuenta(filtracad(model->data(model->index(veces,2),Qt::DisplayRole).toString()))
                 << "} & {\\tiny ";
                stream << filtracad(model->data(model->index(veces,3),Qt::DisplayRole).toString())
                        << "} & {\\tiny ";
                stream << formateanumerosep(importe_factura,coma,decimales);
                stream << "} \\\\ \n  " << "\\hline\n";
                veces++;
              }

    stream << "\\end{longtable}" << "\n";
    stream << "\\end{center}" << "\n";

    stream << "% FIN_CUERPO\n";
    stream << "\\end{document}" << "\n";
    fichero.close();

}


void facturas::consultainforme()
{
    latexinforme();
    QString fichero=tr("documentos");

    int valor=consultalatex2(fichero);
    if (valor==1)
        QMessageBox::warning( this, tr("Informe documentos"),tr("PROBLEMAS al llamar a Latex"));
    if (valor==2)
        QMessageBox::warning( this, tr("Informe documentos"),
                                 tr("PROBLEMAS al llamar a 'dvips'"));
    if (valor==3)
        QMessageBox::warning( this, tr("Informe documentos"),
                              tr("PROBLEMAS al llamar a ")+visordvi());


}


void facturas::consulta()
{
    if (!ui.facstableView->currentIndex().isValid())
    {
     QMessageBox::warning( this, tr("Edición de documentos"),
                           tr("Para editar un documento hay que seleccionarlo de la lista"));
     return;
    }
    QSqlQuery q;
    q = basedatos::instancia()->select_cabecera_doc (serie(), numero());
    if (q.isActive())
       if (q.next())
        {
         factura *f = new factura();
         // "select serie, numero, cuenta, fecha, fecha_fac, fecha_op, "
         // "contabilizado,contabilizable, con_ret, re, tipo_ret, retencion, "
         // "tipo_doc, notas, pie1, pie2, pase_diario_cta "
         f->pasa_cabecera_doc(q.value(0).toString(),
                         q.value(1).toString(),
                         q.value(2).toString(),
                         q.value(4).toDate(),
                         q.value(5).toDate(),
                         q.value(7).toBool(),
                         q.value(8).toBool(),
                         q.value(9).toBool(),
                         q.value(10).toString(),
                         q.value(11).toString(),
                         q.value(12).toString(),
                         q.value(13).toString(),
                         q.value(14).toString(),
                         q.value(15).toString());
         f->activa_consulta();
         f->exec();
         delete(f);
       }

}


void facturas::borraasiento()
{
 if (apunte_diario()=="")
       {
         QMessageBox::warning( this, tr("BORRAR ASIENTO"),
         tr("Para borrar un asiento debe de seleccionar un registro con apunte en el diario"));
         return;
       }
QString elasiento=basedatos::instancia()->selectAsientodiariopase(apunte_diario());
QString ejercicio=ejerciciodelafecha(basedatos::instancia()->select_fecha_diariopase(apunte_diario()));
// consultarasiento(elasiento,usuario,ejercicio);
borrarasientofunc(elasiento,ejercicio);
refresca();
}


void facturas::escondesalir()
{
  // ui.salirpushButton->setEnabled(false);
}
