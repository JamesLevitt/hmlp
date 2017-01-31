n=5000
m=64
s=32
nrhs=128
kmin=21
kmax=22
kinc=16
# =======================================================

echo "@PRIM"
echo 'spdaskit'
# =======================================================

echo "@SETUP"
echo "HMLP_GPU_ARCH = $HMLP_GPU_ARCH"
echo "@SETUP"
echo "HMLP_ARCH = $HMLP_ARCH"
echo "@SETUP"
echo "n = $n"
echo "@SETUP"
echo "m = $m"
echo "@SETUP"
echo "s = $s"
echo "@SETUP"
echo "nrhs = $nrhs"
echo "@SETUP"
echo "kmin = $kmin"
echo "@SETUP"
echo "kmax = $kmax"
echo "@SETUP"
echo "kinc = $kinc"
# =======================================================

echo "@DATE"
date
# =======================================================


for (( k=kmin; k<kmax; k+=kinc ))
do
  ./test_spdaskit.x $n $m $k $s $nrhs; status=$?
  echo "@STATUS"
  echo $status
done
# =======================================================