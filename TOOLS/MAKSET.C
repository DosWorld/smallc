
/*
** makset.c -- make array into a set
*/
makset(array, k, set, size) char array[], set[]; int k, size; {
  int i, j;
  i=k;
  j=0;
  filset(NULL, array, &i, set, &j, size);
  return addset(NULL, set, &j, size);
  }
 return addset(NULL, set, &j, size);
  }
ET], to[DIMSET];
  int allbut, collap, i, lastto;
  if(getarg(1, arg, MAXARR, argc, argv)==EOF) {
    getarg(0, arg, MAXARR, argc, argv);
    printf("?Usage: %s patternfrom [patternto]\n", arg, 1);
    abort(0);
    }
  else if(arg