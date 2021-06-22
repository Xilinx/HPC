np   =  10 ; % Number of points along one dimension
mode =   1 ; % 0: implicit; 1: thermal
tf   = 0.1 ; % Termination time 
dt   = 0.1 ; % Time step

% Grid
tic;
nvtx = np^3 ;
x = zeros(nvtx, 1) ;
y = zeros(nvtx, 1) ;
z = zeros(nvtx, 1) ;
elems = zeros((np-1)^3, 8) ;
d = 1/(np - 1) ;
nelems = 0 ;
ijk = 0 ;
for kk = 1:np
   for jj = 1:np
      for ii = 1:np
         ijk = ijk + 1 ; % ii + (jj-1)*np + (kk-1)*np^2 ;
         x(ijk) = (kk-1)*d ;
         y(ijk) = (jj-1)*d ;
         z(ijk) = (ii-1)*d ;
         if ( ii < np && jj < np && kk < np )
            nelems = nelems + 1 ;
            ijkp   = ijk   + np^2 ; % ii +     (jj-1)*np +     kk*np^2 ;
            ijpk   = ijk   + np ;   % ii +         jj*np + (kk-1)*np^2 ;
            ijpkp  = ijkp  + np ;   % ii +         jj*np +     kk*np^2 ;
            ipjk   = ijk   + 1 ;    % ii + 1 + (jj-1)*np + (kk-1)*np^2 ;
            ipjkp  = ijkp  + 1 ;    % ii + 1 + (jj-1)*np +     kk*np^2 ;
            ipjpk  = ijpk  + 1 ;    % ii + 1 +     jj*np + (kk-1)*np^2 ;
            ipjpkp = ijpkp + 1 ;    % ii + 1 +     jj*np +     kk*np^2 ;
            elems(nelems,:) = [ ijk ijkp ijpkp ijpk ipjk ipjkp ipjpkp ipjpk ] ;
         end
      end
   end
end
fprintf('Grid generation: %f seconds\n',toc) ;

% Keyword file
tic ;
switch (mode)
   case 1
      fn_main = sprintf('ThermalCube%d.k', np) ;
      fn_mesh = sprintf('ThermalCube%d_mesh.k', np) ;
      mid = ' ' ;
      tmid = '1' ;
   otherwise
      fn_main = sprintf('ImplicitCube%d.k', np) ;
      fn_mesh = sprintf('ImplicitCube%d_mesh.k', np) ;
      mid = '1' ;
      tmid = ' ' ;
end
fid=fopen(fn_main, 'w') ;
fprintf(fid, '*KEYWORD\n') ;
fprintf(fid, '*TITLE\n') ;
fprintf(fid, 'Heat transfer\n') ;
fprintf(fid, '*INCLUDE_PATH_RELATIVE\n') ;
fprintf(fid, '.\n') ;
fprintf(fid, '*INCLUDE\n') ;
fprintf(fid, '%s\n', fn_mesh) ;

fprintf(fid, '*PART\n') ;
fprintf(fid, '$      PID     SECID       MID                                              TMID\n') ;
fprintf(fid, 'slab\n') ;
fprintf(fid, '         1         1%10s                                        %10s\n', mid, tmid) ;
fprintf(fid, '*SECTION_SOLID\n') ;
fprintf(fid, '$    SECID    ELFORM\n') ;
fprintf(fid, '         1         1\n') ;
switch (mode)
   case 1
      % Thermal options
      fprintf(fid, '*MAT_THERMAL_ISOTROPIC\n') ;
      fprintf(fid, '$     TMID       TR0\n') ;
      fprintf(fid, '         1       1.0\n') ;
      fprintf(fid, '$       HC        TC\n') ;
      fprintf(fid, '       1.0       1.0\n') ;
      fprintf(fid, '*CONTROL_SOLUTION\n') ;
      fprintf(fid, '$     SOLN\n') ;
      fprintf(fid, '         1\n') ;
      fprintf(fid, '*CONTROL_THERMAL_SOLVER\n') ;
      fprintf(fid, '$    ATYPE     PTYPE    SOLVER\n') ;
      fprintf(fid, '         1         0        12\n') ;
      fprintf(fid, '$   MSGLVL    MAXITR    ABSTOL    RELTOL\n') ;
      fprintf(fid, '         2      5000     1e-16     1e-16\n') ;
      fprintf(fid, '*CONTROL_THERMAL_TIMESTEP\n') ;
      fprintf(fid, '$       TS       TIP       ITS\n') ;
      fprintf(fid, '         0       1.0%10.6f\n', dt) ;
   otherwise
      % Mechanical options
      fprintf(fid, '*MAT_ELASTIC\n') ;
      fprintf(fid, '$      MID        R0         E\n') ;
      fprintf(fid, '         1       1.0       1.0\n') ;
      fprintf(fid, '*CONTROL_IMPLICIT_GENERAL\n') ;
      fprintf(fid, '         1%10.6f\n', dt) ;
      fprintf(fid, '*CONTROL_IMPLICIT_SOLVER\n') ;
      fprintf(fid, '$   LSOLVR    LPRINT\n') ;
      fprintf(fid, '        22         1\n') ;
      fprintf(fid, '$   LCPACK    MTXDMP    MAXITR    ABSTOL    RELTOL\n') ;
      fprintf(fid, '                          5000     1e-16     1e-16\n') ;
end
fprintf(fid, '*CONTROL_TERMINATION\n') ;
fprintf(fid, '$   ENDTIM\n') ;
fprintf(fid, '%10.6f\n', tf) ;
fprintf(fid, '*DATABASE_BINARY_D3PLOT\n') ;
fprintf(fid, '$       DT\n') ;
fprintf(fid, '%10.6f\n', dt) ;
fprintf(fid, '*END\n') ;
fclose(fid) ;

% Geometry and boundary conditions
fid=fopen(fn_mesh, 'w') ;
fprintf(fid, '*NODE\n') ;
fprintf(fid, '$    NID               X               Y               Z\n') ;
for u=1:nvtx
   fprintf(fid, '%8d%16.8e%16.8e%16.8e\n',u,x(u),y(u),z(u)) ;
end
fprintf(fid, '*ELEMENT_SOLID\n') ;
fprintf(fid, '$    EID     PID\n') ;
fprintf(fid, '$     N1      N2      N3      N4      N5      N6      N7      N8\n') ;
for e=1:nelems
   fprintf(fid, '%8d%8d\n',e,1) ;
   fprintf(fid, '%8d%8d%8d%8d%8d%8d%8d%8d\n',elems(e,1:8)) ;
end
switch (mode)
   case 1
      % Prescribed temperature on the {x=0} face
      fprintf(fid, '*SET_NODE_LIST_GENERATE\n') ;
      fprintf(fid, '$      SID\n') ;
      fprintf(fid, '         1\n') ;
      fprintf(fid, '$    B1BEG     B1END\n') ;
      fprintf(fid, '%10d%10d\n', 1, np^2) ;
      fprintf(fid, '*BOUNDARY_TEMPERATURE_SET\n') ;
      fprintf(fid, '$      NID     TLCID     TMULT\n') ;
      fprintf(fid, '         1                 1.0\n', ii) ;
   otherwise
      % Fixed position for the {x=0} face
      fprintf(fid, '*SET_NODE_LIST_GENERATE\n') ;
      fprintf(fid, '$      SID\n') ;
      fprintf(fid, '         1\n') ;
      fprintf(fid, '$    B1BEG     B1END\n') ;
      fprintf(fid, '%10d%10d\n', 1, np^2) ;
      fprintf(fid, '*BOUNDARY_SPC_SET\n') ;
      fprintf(fid, '$      SID       CID      DOFX      DOFY      DOFZ\n') ;
      fprintf(fid, '         1                   1         1         1\n') ;
      % Prescribed compression for the {x=1} face
      fprintf(fid, '*SET_NODE_LIST_GENERATE\n') ;
      fprintf(fid, '$      SID\n') ;
      fprintf(fid, '         2\n') ;
      fprintf(fid, '$    B1BEG     B1END\n') ;
      fprintf(fid, '%10d%10d\n', np^3-np^2+1, np^3) ;
      fprintf(fid, '*DEFINE_CURVE\n') ;
      fprintf(fid, '$     LCID\n') ;
      fprintf(fid, '         1\n') ;
      fprintf(fid, '$                 A1                  O1\n') ;
      fprintf(fid, '                 0.0                1.00\n') ;
      fprintf(fid, '              1.0e10                1.00\n') ;
      fprintf(fid, '*BOUNDARY_PRESCRIBED_MOTION_SET\n') ;
      fprintf(fid, '$   TYPEID       DOF       VAD      LCID\n') ;
      fprintf(fid, '         2         1         0         1\n') ;
end
fclose(fid) ;
fprintf('File generation: %f seconds\n',toc) ;
