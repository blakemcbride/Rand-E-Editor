#!/bin/bash
# shell script to create a new release of Rand editor Version E19
#   update all the release value in specification and build files
# F. Perriollat February 2001

#archive directory
darchiv="./archv"

packg="Rand-E19"
spec="Rand-editor-E19"
rpm="rpm"
rpms="spec"

fmake="Makefile"
fvers="NewVersion"
fnewv="e19/"$fvers

sedscript="sed_script.tmp"

consist_check ()
{
    if [ $1 != $release ]; then
	echo "--- Inconsistancy in '$2' file"
	echo "    (release is internaly assigned to : $1)"
	echo "You must correct this inconsistancy before generate a new version"
	return 1
    fi;
    return 0
}

test_spec ()
{
    if [ $# -gt 1 ]; then
	echo "========================="
	echo "Too many RPM spec files :"
	echo "  '$@'"
	echo "Please remove RPM spec files which are not related to the current release !"
	echo "========================="
	exit 4
    fi
}

test_cc ()
{
    if [ $1 -ne 0 ]; then
	echo "Some error when generating '$2'"
	exit 3
    fi
}

# Get the current release number (from the name of the RPM spec file)
fspec=`ls -m $spec.*.$rpms`
test_spec $fspec
release=${fspec#*.}
release=${release%%.*}

ftar="$packg.${release}.tgz"


if [ $# -gt 0 ]; then
    if test $1 = "-h" -o $1 = "--help"; then
	echo "Synopsis : $0 [-h] [--help] [new-version]"
	echo "    example : \"$0 +1\" : new release, revision nb incremented by 1"
	echo
	echo "Script to generate a new version of Rand editor."
	echo "    $fmake, RPM specification ($fspec),"
	echo "    and shell script '$fnewv' will be updated."
	echo
	echo "  If no tar archive file ($ftar} of the current version exist,"
	echo "    it will be generated".
	echo
	echo "  Without parameter, a consistancy chech is done."
	echo
	exit 0;
    else
	if [ $# -gt 1 ]; then
	    echo $2 : Invalid parameter
	    echo synopsis : "$0 [-h] [--help]"
	    exit 1;
	fi;
    fi;
fi;

specr=`sed -n "/^Release:/ s/^Release: *//p" $fspec`
maker=`sed -n "/^RELEASE=/ s/^RELEASE=//p" $fmake`
newvr=`sed -n "/^release/ s/^release[^-]*-\([0-9]*\).*/\1/p" $fnewv`

echo
echo "Current Rand editor E19 release is : $release"

# check current version for consistancy
consiserr="no"
consist_check $specr $fspec
if [ $? -ne 0 ]; then consiserr="yes"; fi
consist_check $maker $fmake
if [ $? -ne 0 ]; then consiserr="yes"; fi
consist_check $newvr $fnewv
if [ $? -ne 0 ]; then consiserr="yes"; fi
if [ $consiserr != "no" ]; then
    exit 1;
fi

# just a check ?
if [ $# -eq 0 ]; then
    echo "Consistancy check succefully completed"
    echo
    exit 0;
fi

let rl=$release

# build the new release value
delta=${1%%[0-9]*}
val=${1#+*}

if [ -n "$delta" ]; then
    if [ $delta != "+" ]; then
	echo "Invalide new version increment value : '$1'"
	echo "    valide value must be flag with a single '+'."
	exit 2;
    fi;
    if [ -z "$val" ]; then
	val=1
    fi
    let nrl=$rl+$val
else
    let nrl=$val
fi;

if [ $nrl -le $rl ]; then
    echo "I cannot build a version $nrl less or equal to the current one $rl"
    exit 3;
fi;

newfspec=$spec.$nrl.$rpms

echo "Now I will create the $packg new release $nrl"
echo "  Before I will build a tar archive '$ftar'"
echo "  of the current version"

if [ -e $ftar ]; then
    echo "  Before I will build a tar archive '$ftar'"
    echo "  of the current version"
    mkdir -p $darchiv
    echo "  I will move a previous tar archive it into '$darchiv' directory"
    if [ -e $darchiv/$ftar ]; then
	mv -f $darchiv/$ftar $darchiv/$ftar.old
    fi;
    mv $ftar $darchiv
    echo "  Now I build the tar archive 'ftar'"
    gmake tar
fi;


# Update the e19/NewVersion script file
echo "1 - Now I update $fnewv"
rm -f $fnewv.new
sed -e "/^release=/ s/\(^release=.*:-\)\([0-9]*\)\(.*\)/\1$nrl\3/" \
    -e "s/\(.* revision nb (default \)\([0-9]*\)\(.*\)/\1$nrl\3/" \
    $fnewv > $fnewv.new
test_cc $? $fnewv.new


# Update the Makefile
echo "2 - Now I update $fmake"
rm -f $fmake.new
sed -e "/^RELEASE=/ s/\(^RELEASE=\).*/\1$nrl/" \
    -e "/.*$spec\.[0-9]*\.$rpms/ s/\(.*\)$spec\.[0-9]*\.$rpms\(.*\)/\1$newfspec\2/" \
    $fmake > $fmake.new
test_cc $? $fmake.new


# Update the RPM spec file
echo "3 - Now I will generate the RPM spec file"
rm -f $newfspec

rootuser="perrioll"
string1b=`whoami`
string1c=`hostname`
if test $string1b = "root";
    then string1b=$rootuser;
fi;
string1="${string1b}@${string1c}.cern.ch"
comment="==== comment to be put there ===="

echo " 3.1 - Now I build the sed script file '$sedscript'"
rm -f $sedscript

cat > $sedscript <<-EOF
    /^Release: / s/\(^Release: \).*/\1$nrl/
    /.*$spec\.[0-9]*\.$rpms/ s/\(.*\)$spec\.[0-9]*\.$rpms\(.*\)/\1$newfspec\2/
    /.*$packg-[0-9]*\.i386\.$rpm.*/ s/\(.*\)\($packg-\)[0-9]*\(\.i386\.$rpm.*\)/\1\2$nrl\3/
    /^%changelog/a\\
* $(date '+%a %b %d %Y') by $string1\\
- revision $nrl\\
- $comment
EOF

echo " 3.2 - Now I generate RPM spec file '$newfspec'"
sed -f $sedscript $fspec > $newfspec

# NB : the "-f - ... <<-EOF" does not work on AIX !!!
# sed -f - $fspec > $newfspec <<-EOF
#     /^Release: / s/\(^Release: \).*/\1$nrl/
#     /.*$spec\.[0-9]*\.$rpms/ s/\(.*\)$spec\.[0-9]*\.$rpms\(.*\)/\1$newfspec\2/
#     /.*$packg-[0-9]*\.i386\.$rpm.*/ s/\(.*\)\($packg-\)[0-9]*\(\.i386\.$rpm.*\)/\1\2$nrl\3/
#     /^%changelog/a\\
# * $(date '+%a %b %d %Y') by $string1\\
# - revision $nrl\\
# - $comment
# EOF

test_cc $? $newfspec

rm -f $sedscript

echo "Do not foreget to edit '$newfspec' for the actual revision comment lines"
echo "  the generated line is '$comment'"


# Overwrite old files by new one
mkdir -p $darchiv
mv -f $fspec $darchiv
mv -f $fnewv $darchiv/${fvers}.${release}
mv -f $fmake $darchiv/${fmake}.${release}
mv -f $fnewv.new $fnewv
mv -f $fmake.new $fmake

# Well done !
exit 0
