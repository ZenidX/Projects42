$urls = @{
  "c02" = "https://cdn.intra.42.fr/pdf/pdf/213035/es.subject.pdf"
  "c03" = "https://cdn.intra.42.fr/pdf/pdf/216262/es.subject.pdf"
  "c04" = "https://cdn.intra.42.fr/pdf/pdf/211885/es.subject.pdf"
  "c05" = "https://cdn.intra.42.fr/pdf/pdf/211984/es.subject.pdf"
  "c06" = "https://cdn.intra.42.fr/pdf/pdf/212273/es.subject.pdf"
  "c07" = "https://cdn.intra.42.fr/pdf/pdf/220317/es.subject.pdf"
  "c08" = "https://cdn.intra.42.fr/pdf/pdf/165980/es.subject.pdf"
  "c09" = "https://cdn.intra.42.fr/pdf/pdf/165982/es.subject.pdf"
  "c10" = "https://cdn.intra.42.fr/pdf/pdf/165984/es.subject.pdf"
  "c11" = "https://cdn.intra.42.fr/pdf/pdf/132673/es.subject.pdf"
  "c12" = "https://cdn.intra.42.fr/pdf/pdf/209694/es.subject.pdf"
  "c13" = "https://cdn.intra.42.fr/pdf/pdf/211295/es.subject.pdf"
}
foreach ($m in $urls.Keys | Sort-Object) {
  $dir = "E:\WORK\Xavi\Projects42\c\$m"
  New-Item -ItemType Directory -Force $dir | Out-Null
  Invoke-WebRequest $urls[$m] -OutFile "$dir\subject.pdf"
  python -c "from pypdf import PdfReader; import sys; [print(f'--- PAGINA {i+1} ---\n' + (p.extract_text() or '')) for i,p in enumerate(PdfReader(r'$dir\subject.pdf').pages)]" | Out-File "$dir\subject.txt" -Encoding utf8
  $lines = (Get-Content "$dir\subject.txt" | Measure-Object -Line).Lines
  Write-Output "$m : $lines lineas"
}
