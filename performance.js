import http from 'k6/http'
import { check } from 'k6'

export default function () {
  const headers = {
    "Content-Type": "application/json",
  }
  let res = http.get('http://localhost/petr4/2015-01-01/2024-01-01', { headers: headers })
  check(res, { 'success login': (r) => r.status === 200 })
}
export const options = {
  insecureSkipTLSVerify: true,
}
