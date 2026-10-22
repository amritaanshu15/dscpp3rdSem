
var Api = {
  token: sessionStorage.getItem('eco_token'),
  user: sessionStorage.getItem('eco_user'),

  async call(method, url, body) {
    var options = { method: method, headers: { 'Content-Type': 'application/json' } };
    if (this.token) {
      options.headers['x-auth-token'] = this.token;
    }
    if (body !== undefined) {
      options.body = JSON.stringify(body);
    }

    var response;
    try {
      response = await fetch(url, options);
    } catch (e) {
      throw new Error('Cannot reach the server. Is ecolife_engine running?');
    }

    var data;
    try {
      data = await response.json();
    } catch (e) {
      data = { ok: false, error: 'Bad answer from the server' };
    }

    // token not valid any more (the program was restarted)
    if (response.status === 401 && this.token) {
      this.clearSession();
      location.reload();
    }
    if (!response.ok || data.ok === false) {
      throw new Error(data.error || 'Request failed');
    }
    return data;
  },

  get(url)        { return this.call('GET', url); },
  post(url, body) { return this.call('POST', url, body || {}); },
  put(url, body)  { return this.call('PUT', url, body || {}); },
  del(url)        { return this.call('DELETE', url); },

  saveSession(token, user) {
    this.token = token;
    this.user = user;
    sessionStorage.setItem('eco_token', token);
    sessionStorage.setItem('eco_user', user);
  },

  clearSession() {
    this.token = null;
    this.user = null;
    sessionStorage.removeItem('eco_token');
    sessionStorage.removeItem('eco_user');
  }
};
