declare -a filearray=(
"/workspace/biros/sc17/data_to_use_65K/K02N65536.bin"
"/workspace/biros/sc17/data_to_use_65K/K03N65536.bin"
"/workspace/biros/sc17/data_to_use_65K/K04N65536.bin"
"/workspace/biros/sc17/data_to_use_65K/K05N65536.bin"
"/workspace/biros/sc17/data_to_use_65K/K06N65536.bin"
"/workspace/biros/sc17/data_to_use_65K/K07N65536.bin"
)
n=65536
m=512
s=512
k=32
stol=1E-4
budget=0.03
nrhs=512
# ======= Do not change anything below this line ========

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
echo "k = $k"
echo "@SETUP"
echo "stol = $stol"
echo "@SETUP"
echo "budget = $budget"
echo "@SETUP"
echo "nrhs = $nrhs"
# =======================================================

echo "@DATE"
date
# =======================================================

for filename in "${filearray[@]}"
do
  echo $filename
  ./test_spdaskit.x $n $m $k $s $nrhs $stol $budget $filename; status=$?
  echo "@STATUS"
  echo $status
done

# =======================================================