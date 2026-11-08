// Suporte do navegador (Web Serial + contexto seguro)
(function () {
  var box = document.getElementById('aviso');
  var temSerial = 'serial' in navigator;
  var seguro = window.isSecureContext;
  if (temSerial && seguro) {
    box.className = 'aviso-navegador ok';
    box.innerHTML = '<span>✅</span><div>Seu navegador tem tudo o que precisa. '
      + 'Conecte a placa e clique em <b>Instalar</b>.</div>';
  } else if (temSerial && !seguro) {
    box.innerHTML = '<span>🔒</span><div>Abra este site por <b>HTTPS</b> (GitHub Pages) '
      + 'ou por <b>localhost</b> para poder gravar.</div>';
  } else {
    box.innerHTML = '<span>⚠️</span><div>Este navegador não tem <b>Web Serial</b>. '
      + 'Use o <b>Chrome</b>, <b>Edge</b> ou <b>Opera</b> no computador (não funciona no celular, Firefox ou Safari).</div>';
  }
})();

// Abas do preview
(function () {
  var legendas = {
    menu: 'Menu inicial: um botão por jogo, repetido para os dois lados da mesa.',
    rift: 'Riftbound: pontos até 8, melhor de 3 (bolinhas) e cronômetro no meio.',
    mtg:  'Magic: vida inicial 20/30/40, ±1 e ±5, variação recente e veneno.'
  };
  var abas = document.querySelectorAll('.abas button');
  var views = document.querySelectorAll('.tela-view');
  var leg = document.getElementById('legenda-preview');
  abas.forEach(function (b) {
    b.addEventListener('click', function () {
      var alvo = b.getAttribute('data-tab');
      abas.forEach(function (x) { x.setAttribute('aria-selected', x === b ? 'true' : 'false'); });
      views.forEach(function (v) { v.classList.toggle('ativa', v.getAttribute('data-view') === alvo); });
      leg.textContent = legendas[alvo] || '';
    });
  });
})();

// SHA-256 dos binários publicados (gerado pela CI / servir_local.py), para
// conferir o que vai ser gravado.
(function () {
  document.querySelectorAll('.sha[data-sums]').forEach(function (el) {
    fetch(el.getAttribute('data-sums'), { cache: 'no-store' })
      .then(function (r) { if (!r.ok) throw new Error(r.status); return r.text(); })
      .then(function (txt) {
        var linhas = txt.trim().split('\n').filter(Boolean);
        if (!linhas.length) throw new Error('vazio');
        el.textContent = 'SHA-256:';
        linhas.forEach(function (l) {
          var m = l.match(/^([0-9a-f]{64})\s+\*?(.+)$/);
          if (!m) return;
          var d = document.createElement('div');
          var nome = document.createElement('span');
          nome.textContent = m[2] + ' ';
          var h = document.createElement('code');
          h.textContent = m[1];
          d.appendChild(nome); d.appendChild(h);
          el.appendChild(d);
        });
      })
      .catch(function () {
        el.textContent = 'SHA-256 indisponível (binário ainda não publicado).';
      });
  });
})();
