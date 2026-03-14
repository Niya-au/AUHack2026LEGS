## Credentials

| Key | Value |
|-----|-------|
| `supabase_url` | `https://sefoaaasnyorakarqsrm.supabase.co` |
| `supabase_anon_key` | `eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InNlZm9hYWFzbnlvcmFrYXJxc3JtIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NzM0MTE2NDYsImV4cCI6MjA4ODk4NzY0Nn0.mfGpP7xW3b5rHXFtck1ujOGvyHUbiMaWoN8vhdru8Vg` |

---

## Endpoints

### Poll for armed leg

**POST** `/rest/v1/rpc/is_leg_armed`

Returns an empty array if no leg is armed, or the row if one is waiting.

```
POST https://sefoaaasnyorakarqsrm.supabase.co/rest/v1/rpc/is_leg_armed
Body: {}
```

Response (armed):
```json
[{ "id": 5, "username": "alice" }]
```

Response (not armed):
```json
[]
```

---

### Submit measured time

**POST** `/rest/v1/rpc/submit_leg_time`

Writes the measured time (ms) to whichever row is "armed".
Returns `true` if updated, `false` if no armed leg existed at time of call.

```
POST https://sefoaaasnyorakarqsrm.supabase.co/rest/v1/rpc/submit_leg_time
Body: { "p_time": 1234 }
```

---

## Required headers

All requests need:

```
apikey: <supabase_anon_key>
Authorization: Bearer <supabase_anon_key>
Content-Type: application/json
```

---

### Arm a leg (called from web UI)

**POST** `/rest/v1/rpc/arm_leg`

Clears any existing armed leg and creates a new one for the given user.

```
POST https://sefoaaasnyorakarqsrm.supabase.co/rest/v1/rpc/arm_leg
Body: { "p_username": "alice" }
```

Response: the new row `id` as a number.
