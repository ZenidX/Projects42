// Índice lateral: marca la sección visible y, en pantallas estrechas, añade
// un botón "Índice" que abre y cierra la barra como panel.
(function () {
  var toc = document.querySelector('.toc');
  if (!toc) return;
  document.body.classList.add('has-toc');

  var btn = document.createElement('button');
  btn.className = 'toc-btn';
  btn.type = 'button';
  btn.textContent = '☰ Índice';
  btn.setAttribute('aria-label', 'Abrir el índice');
  document.body.appendChild(btn);
  function close() { document.body.classList.remove('toc-open'); btn.textContent = '☰ Índice'; }
  btn.addEventListener('click', function () {
    var open = document.body.classList.toggle('toc-open');
    btn.textContent = open ? '✕ Cerrar' : '☰ Índice';
  });
  toc.addEventListener('click', function (e) { if (e.target.closest('a')) close(); });
  document.addEventListener('keydown', function (e) { if (e.key === 'Escape') close(); });
  document.addEventListener('click', function (e) {
    if (document.body.classList.contains('toc-open') && !toc.contains(e.target) && e.target !== btn) close();
  });

  var links = Array.prototype.slice.call(toc.querySelectorAll('a[href^="#"]'));
  var targets = links.map(function (a) {
    return document.getElementById(decodeURIComponent(a.getAttribute('href').slice(1)));
  });
  var last = -2;
  function update() {
    var y = window.scrollY + window.innerHeight * 0.25;
    var current = -1;
    for (var i = 0; i < targets.length; i++) {
      if (targets[i] && targets[i].getBoundingClientRect().top + window.scrollY <= y) current = i;
    }
    if (current === last) return;
    last = current;
    links.forEach(function (a, i) { a.classList.toggle('current', i === current); });
    if (current >= 0 && window.innerWidth >= 900) {
      var a = links[current], r = a.getBoundingClientRect(), t = toc.getBoundingClientRect();
      if (r.top < t.top + 40 || r.bottom > t.bottom - 40)
        toc.scrollTop += (r.top + r.height / 2) - (t.top + t.height / 2);
    }
  }
  window.addEventListener('scroll', update, { passive: true });
  window.addEventListener('resize', update);
  update();
})();
