/* $Id: ScreenWidget.cpp,v 1.41 2002/03/15 16:03:53 aspert Exp $ */


/*
 *
 *  Copyright (C) 2001-2002 EPFL (Swiss Federal Institute of Technology,
 *  Lausanne) This program is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 *  USA.
 *
 *  In addition, as a special exception, EPFL gives permission to link
 *  the code of this program with the Qt non-commercial edition library
 *  (or with modified versions of Qt non-commercial edition that use the
 *  same license as Qt non-commercial edition), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt non-commercial edition.  If you modify this file, you may extend
 *  this exception to your version of the file, but you are not
 *  obligated to do so.  If you do not wish to do so, delete this
 *  exception statement from your version.
 *
 *  Authors : Nicolas Aspert, Diego Santa-Cruz and Davy Jacquet
 *
 *  Web site : http://mesh.epfl.ch
 *
 *  Reference :
 *   "MESH : Measuring Errors between Surfaces using the Hausdorff distance"
 *   Submitted to ICME 2002, available on http://mesh.epfl.ch
 *
 */


    return;
  }
}

void ScreenWidget::trapChanges(int n) 
{
  int slv = qslidDispSampDensity->value(); // value of the slider
  int spv = qspSampDensity->value(); // value of the spinbox
  bool hasChanged = FALSE;

  if (slv == n && spv == n)
    return;

  if (slv != n) {
    qslidDispSampDensity->setValue(n);
    hasChanged = TRUE;
  }

  if (spv != n) {
    qspSampDensity->setValue(n);
    hasChanged = TRUE;
  }
  
  if (hasChanged) {
    changeGroupBoxTitle(n);
    emit dsValChange(n);  
  }
}

void ScreenWidget::updatecbStatus(bool state) {
  if (!state) { // no light
    qcbTwoSide->setDisabled(TRUE);
    qcbInvNorm->setDisabled(TRUE);
  } else {
    qcbTwoSide->setDisabled(FALSE);
    qcbInvNorm->setDisabled(FALSE);
  }
    
}

void ScreenWidget::aboutMesh()
{
  QString msg;

  msg.sprintf("Mesh v %s\n"
              "Copyright (C) %s\n"
              "Authors: Nicolas Aspert, Diego Santa Cruz, Davy Jacquet\n",
              version, copyright);
  QMessageBox::about(this, "Mesh", msg);
}

void ScreenWidget::aboutKeys()
{
    QMessageBox::about( this, "Key bindings",
			"F1: Toggle Wireframe/Fill\n"
			"F2: Toggle lighting (right model only)\n"
			"F3: Toggle viewpoint synchronization\n"
			"F4: Invert normals (right model only)\n"
                        "F5: Toggle two sided material (right model only)");
}

void ScreenWidget::aboutBugs()
{
    QMessageBox::about( this, "Bug",
			"If you found a bug, please send an e-mail to :\n"
			"Nicolas.Aspert@epfl.ch or\n"
			"Diego.SantaCruz@epfl.ch");
}

void ScreenWidget::quit()
{
  QApplication::exit(0);
}


