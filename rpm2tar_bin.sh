#!/bin/bash
#
# Build a binary compressed tar file from the rpm package (binary).
#   Include a README-install documentation file (extracted from
#   rpm information.
#
# Fabien Perriollat - CERN - January 2002
#

synopsis="synopsis :\n$0 [-h] [--help] [--verbose] <rpm-binary-pacakage-file-name> [build-path]"

yes="yes"
verbose="no"

help ()
{
    if [ $# -ge 1 ]; then
	echo -e "\n$1";
    fi
    echo -e $synopsis;
    exit ${2:-0}
}


if  [ $# -lt 1 -o ${1:-no} = "--help" -o ${1:-no} = "-h" ]; then
    if [ $# -ge 1 ]; then
	help "Script to build a tar file from a rpm (binary) package file" 1;
    else
	help "$0 : Too few paramaters" 1;
    fi
fi;

if [ $1 = "--verbose" ]; then
    verbose=$yes
    shift 1
fi;

if [ $# -gt 2 -o $# -lt 1 ]; then
    if [ $# -lt 1 ]; then
	help "Too few parameters" 2;
    else
	help "Invalid arguments \"$*\"" 2;
    fi
fi;

# the default working directory is /tmp
path=${2:-/tmp}

if [ ! -e $1 ]; then
    echo -e "\a\nThe file : \"$1\"\n    does not exist, nothing can be done !\n"
    exit 1
fi
if [ ! -r $1 ]; then
    echo -e "\a\nThe file \"$1\"\n    cannot be read, nothing can be done !\n"
    exit 1
fi

/* Get package name and relocation directory */
pname=`rpm -qp --qf "%{NAME}-%{VERSION}-%{RELEASE}\n" "$1"`

if [ $? -ne 0 ]; then
    echo -e "\a\n$0 : File error\n    The file : \"$1\"\n    is not a rpm package file, nothing can be done !\n"
    exit 1
fi
case $1 in
    *.src.*)
	echo -e "\a\n$0 : File error\n    The file : \"$1\"\n    seems to be the rpm source package file of \"${pname}\",\n    nothing can be done !\n"
	exit 1;;
esac

reloc_val=`rpm -qp --qf "%{PREFIXES}" "$1"`
reloc=${reloc_val:-/.}

# build the compressed tar file name
# fn=${1##*/}
# tarfile="${fn%.rpm}.tgz"
tarfile="`basename $1 .rpm`.tgz"

# Where to build the target file
dir="${path}/${pname}"
fil_dir="${dir}/files"
pckg_dir=${fil_dir}${reloc}
read_me="${pckg_dir}/README-${pname:-xxxx}"

echo
echo "Package : ${pname}"
echo "Target : ${dir}/${tarfile}"
echo

if [ ! -d $path ]; then
    echo -e "\a\n$0 : ${dir}\n    is not a directory, nothing can be done !\n"
    exit 2
fi

if [ -d $dir ]; then
    echo -e "Clean working directory : ${dir}\n"
    rm -Rf "${dir}"
else
    mkdir -p $dir
fi

mkdir -p $fil_dir

echo -e "Extract the package into ${fil_dir}\n"
rpm2cpio $1 | (cd $fil_dir; cpio -idm)

echo -e "Build the Readme file : ${read_me}\n"

cat > ${read_me} <<EOF
Compressed tar file : "${tarfile}" build
    `date`

Package : $pname
    `rpm -qp --qf "%{SUMMARY}\n" "$1"`
    Subversion : `rpm -qp --qf "%{SERIAL}\n" "$1"`
    url : `rpm -qp --qf "%{URL}\n" "$1"`
    The compressed tar archive was build from Rpm bin package file :
    "$1"

Rpm bin package file was build on : `rpm -qp --qf "%{BUILDHOST}\n" "$1"`
    at  : `rpm -qp --qf "%{BUILDTIME:date}\n" "$1"`
    for : `rpm -qp --qf "%{OS} %{ARCH}\n" "$1" `
    by  : `rpm -qp --qf "%{PACKAGER}\n" "$1"`

Require :
=========
`rpm -qp --qf "[%{REQUIRENAME} %{REQUIREFLAGS:depflags} %{REQUIREVERSION}\n]" "$1" | sed -n "\:^rpmlib:! s:^:  : p"`

Package files :
===============

`rpm -qlp "$1" | sed "\:^${reloc}: s:^${reloc}:  .:"`

Description :
=============
`rpm -qp --qf "%{DESCRIPTION}\n" "$1"`

Change log :
============
`rpm -qp --qf "[* %{CHANGELOGTIME:date} %{CHANGELOGNAME}\n%{CHANGELOGTEXT}\n\n]\n" "$1"`

EOF

# Now build the tar file
cmd="(cd ${pckg_dir}; tar czf ${dir}/${tarfile} .)"
echo -e "Executing : \n$cmd"
eval ${cmd}

echo -e "\nCompressed tar file :"
ls -l ${dir}/${tarfile}

# Clean the temporary file space
rm -Rf ${fil_dir}
if [ -d $fil_dir ]; then
    rmdir ${fil_dir}
fi

if [ ${verbose} = $yes ]; then
    echo -e "\nArchive content :\n";
    tar -tvzf ${dir}/${tarfile};
fi

echo -e "\n---- $0 completed ----"

exit 0
