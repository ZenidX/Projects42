// Resalta en el índice (ul.toc) el enlace de la sección que se está leyendo.
(function () {
  var links = Array.prototype.slice.call(document.querySelectorAll('.toc a[href^="#"]'));
  var targets = links.map(function (a) {
    return document.getElementById(decodeURIComponent(a.getAttribute('href').slice(1)));
  });
  if (!links.length) return;
  function update() {
    var y = window.scrollY + window.innerHeight * 0.25;
    var current = -1;
    for (var i = 0; i < targets.length; i++) {
      if (targets[i] && targets[i].getBoundingClientRect().top + window.scrollY <= y) current = i;
    }
    links.forEach(function (a, i) { a.classList.toggle('current', i === current); });
  }
  window.addEventListener('scroll', update, { passive: true });
  window.addEventListener('resize', update);
  update();
})();
