# Spec file for MESH
# $Id: Mesh.spec,v 1.13 2004/08/31 20:37:26 aspert Exp $
Summary: Measuring Errors between Surfaces using the Hausdorff distance
Name: Mesh
Version: 1.12
Release: 1
License: GPL
Group: Applications/Scientific Visualization
Source: http://mesh.epfl.ch/Mesh-1.12.tar.gz
URL: http://mesh.epfl.ch
Packager: Nicolas Aspert <Nicolas.Aspert@epfl.ch>
Requires: qt >= 2.3, zlib >= 1.1, libpng >= 1, libjpeg >= 6, Mesa >= 3.4, libstdc++ >= 2.95
BuildRequires: qt-devel >= 2.3, zlib-devel >= 1.1
AutoReqProv: no
Prefix: %{_prefix}

%description
MESH is a tool that measures distortion between two discrete surfaces
(triangular meshes). It uses the Hausdorff distance to compute a
maximum, mean and root-mean-square errors between two given
surfaces. Besides providing figures, MESH can also display the error
values on the surface itself.  The source code of MESH is available
under the GNU General Public License. It requires the Qt library
(available for free), and has been successfully tested on several
platforms.

# disable debug package generation (troubles with moc files)
%define debug_package %{nil}

%prep
%setup

%build
make all

%install
install -D $RPM_BUILD_DIR/%{name}-%{version}/bin/mesh %{_bindir}/mesh

%files
%defattr(-,root,root)
%doc README
%{_bindir}/mesh
