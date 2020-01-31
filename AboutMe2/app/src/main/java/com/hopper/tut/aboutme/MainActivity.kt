package com.hopper.tut.aboutme

import android.content.Context
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import android.view.inputmethod.InputMethodManager
import android.widget.Button
import android.widget.EditText
import android.widget.TextView

class MainActivity : AppCompatActivity() {

    private lateinit var nnEdit: EditText
    private lateinit var nnLabel: TextView
    private lateinit var doneBtn: Button

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        nnEdit  = findViewById<EditText>(R.id.nickname_edit)
        nnLabel = findViewById<TextView>(R.id.nickname_text)
        doneBtn = findViewById<Button>(R.id.done_button)

        doneBtn.setOnClickListener { addNickname() }
        nnLabel.setOnClickListener { removeNickname() }
    }

    private fun addNickname() {
        if (nnEdit.text.isBlank()) {
            if (nnEdit.text.isNotEmpty())
                nnEdit.setText("")
        }
        else {
            nnEdit.visibility = View.GONE
            doneBtn.visibility = View.GONE
            nnLabel.text = nnEdit.text.trim()
            nnLabel.visibility = View.VISIBLE

            // Hide the keyboard:
            val inputMethodManager = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
            inputMethodManager.hideSoftInputFromWindow(doneBtn.windowToken, 0)
        }
    }

    private fun removeNickname() {
        nnEdit.setText("")
        nnEdit.visibility = View.VISIBLE
        doneBtn.visibility = View.VISIBLE
        nnLabel.visibility = View.GONE
        nnEdit.requestFocus()

        // Show the keyboard:
        val imm = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        imm.showSoftInput(nnEdit, 0)
    }
}
