/* Copyright (c) Bureau d'Etudes Ciaran O'Donnell,1987,1990,1991 */
/*
 * C library -- setpgrp, getpgrp
 */


getpgrp() {

	return(_pgrp(0));
}


setpgrp() {

	return(_pgrp(1));
}
