#
# $Id$
#

# This file is read by the (bsd package) Makefile

package_build = 1
package_category = misc
package_ext = tbz

OS != uname

.if ${OS} == OpenBSD
        package_ext = tgz       
.endif


